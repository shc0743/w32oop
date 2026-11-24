#include "./LayoutManager.hpp"
#include "./Widget.hpp"
#include <utility>

using namespace std;
using namespace w32oop;
using namespace w32oop::ui;

w32oop::ui::LayoutManager::LayoutManager()
	: root_box_(make_unique<ui::FlexLayout>()) {
}

w32oop::ui::LayoutManager::~LayoutManager() { detach(); }

void w32oop::ui::LayoutManager::setRenderer(std::function<VNode()> renderer) {
	renderer_ = std::move(renderer);
	invalidate();
}

void w32oop::ui::LayoutManager::renderNow() {
	ensure_rendered();
}

void w32oop::ui::LayoutManager::attach(ui::Window* host) {
	host_ = host;
	reconciler_.reset();
	if (renderer_) invalidate();
}

void w32oop::ui::LayoutManager::detach() {
	if (root_box_) root_box_->clear();
	reconciler_.reset();
	host_ = nullptr;
}

void w32oop::ui::LayoutManager::requestLayout() {
	if (!host_ || !host_->created()) return;
	const HWND hwnd = static_cast<HWND>(*host_);
	if (!hwnd || !IsWindow(hwnd)) return;
	const RECT rc = host_->client_rect();
	ui::LayoutRect area{ 0, 0, rc.right - rc.left, rc.bottom - rc.top };
	if (area.width <= 0 || area.height <= 0) return;
	for (const ui::Placement& p : arrange(area)) {
		if (p.window && p.window->created())
			p.window->resize(p.rect.x, p.rect.y, p.rect.width, p.rect.height);
	}
}

ui::Window* w32oop::ui::LayoutManager::find(const std::wstring& key) const {
	return reconciler_ ? reconciler_->find(key) : nullptr;
}

size_t w32oop::ui::LayoutManager::mounted_count() const {
	return reconciler_ ? reconciler_->mounted_count() : 0;
}

void w32oop::ui::LayoutManager::ensure_rendered() {
	if (!render_dirty_ || !renderer_ || !host_) return;

	if (!reconciler_) {
		reconciler_ = make_unique<internal::Reconciler>(*host_, *root_box_);
	}

	int guard = 0;
	do {
		render_dirty_ = false;
		rendering_ = true;
		try {
			VNode tree;
			{
				EffectScope scope(this);
				tree = renderer_();
			}

			const auto* descriptor = descriptor_of(tree);
			if (!descriptor || !descriptor->is_container) {
				VNode wrapper = internal::make_node<ui::FlexLayout>();
				wrapper.children.push_back(std::move(tree));
				tree = std::move(wrapper);
			}

			reconciler_->render(tree);
			rendering_ = false;
		}
		catch (...) {
			rendering_ = false;
			throw;
		}
	} while (render_dirty_ && ++guard < 10);
}

ui::LayoutSize w32oop::ui::LayoutManager::measure(ui::LayoutSize available) const {
	const_cast<LayoutManager*>(this)->ensure_rendered();
	return root_box_ ? root_box_->measure(available) : ui::LayoutSize{};
}

vector<ui::Placement> w32oop::ui::LayoutManager::arrange(const ui::LayoutRect& area) {
	ensure_rendered();
	return root_box_->arrange(area);
}

void w32oop::ui::LayoutManager::invalidate() {
	render_dirty_ = true;
	if (rendering_) return;
	if (!host_ || !host_->created()) return;
	const HWND hwnd = static_cast<HWND>(*host_);
	if (!hwnd || !IsWindow(hwnd)) return;
	const RECT rc = host_->client_rect();
	host_->post(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));
}

void w32oop::ui::LayoutManager::on_changed() {
	invalidate();
}

void w32oop::ui::LayoutManager::set_reflow_handler(std::function<void()> handler) {
	reflow_handler_ = std::move(handler);
}
