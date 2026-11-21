#include "./FlexLayout.hpp"
#include "./Window.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

using namespace std;
using namespace w32oop::ui;

namespace {
	LayoutSize to_layout_size(SIZE size) {
		return LayoutSize{ static_cast<int>(size.cx), static_cast<int>(size.cy) };
	}
}

// ---------------------------------------------------------------------------
// FlexItem
// ---------------------------------------------------------------------------

FlexItem::FlexItem(Window& child, FlexLayout& owner) noexcept
	: owner_(owner), window_(&child) {
}

FlexItem::FlexItem(LayoutNode& child, FlexLayout& owner) noexcept
	: owner_(owner), node_(&child) {
}

FlexItem& FlexItem::grow(float value) {
	spec_.grow = value;
	owner_.invalidate();
	return *this;
}

FlexItem& FlexItem::shrink(float value) {
	spec_.shrink = value;
	owner_.invalidate();
	return *this;
}

FlexItem& FlexItem::basis(int pixels) {
	spec_.basis = Length(pixels);
	owner_.invalidate();
	return *this;
}

FlexItem& FlexItem::basis(Percent percent) {
	spec_.basis = Length(percent);
	owner_.invalidate();
	return *this;
}

FlexItem& FlexItem::width(int pixels) {
	spec_.width = Length(pixels);
	owner_.invalidate();
	return *this;
}

FlexItem& FlexItem::width(Percent percent) {
	spec_.width = Length(percent);
	owner_.invalidate();
	return *this;
}

FlexItem& FlexItem::height(int pixels) {
	spec_.height = Length(pixels);
	owner_.invalidate();
	return *this;
}

FlexItem& FlexItem::height(Percent percent) {
	spec_.height = Length(percent);
	owner_.invalidate();
	return *this;
}

FlexItem& FlexItem::size(int width, int height) {
	spec_.width = Length(width);
	spec_.height = Length(height);
	owner_.invalidate();
	return *this;
}

FlexItem& FlexItem::minWidth(int pixels) {
	spec_.minWidth = pixels;
	owner_.invalidate();
	return *this;
}

FlexItem& FlexItem::maxWidth(int pixels) {
	spec_.maxWidth = pixels;
	owner_.invalidate();
	return *this;
}

FlexItem& FlexItem::minHeight(int pixels) {
	spec_.minHeight = pixels;
	owner_.invalidate();
	return *this;
}

FlexItem& FlexItem::maxHeight(int pixels) {
	spec_.maxHeight = pixels;
	owner_.invalidate();
	return *this;
}

FlexItem& FlexItem::margin(int all) {
	spec_.margin = LayoutInsets::all(all);
	owner_.invalidate();
	return *this;
}

FlexItem& FlexItem::margin(LayoutInsets insets) {
	spec_.margin = insets;
	owner_.invalidate();
	return *this;
}

FlexItem& FlexItem::alignSelf(Align align) {
	spec_.alignSelf = align;
	owner_.invalidate();
	return *this;
}

// ---------------------------------------------------------------------------
// FlexLayout
// ---------------------------------------------------------------------------

FlexLayout::FlexLayout(Direction direction)
	: direction_(direction) {
}

FlexLayout::~FlexLayout() = default;

FlexLayout& FlexLayout::direction(Direction value) {
	direction_ = value;
	invalidate();
	return *this;
}

FlexLayout& FlexLayout::wrap(bool value) {
	wrap_ = value;
	invalidate();
	return *this;
}

FlexLayout& FlexLayout::justifyContent(Justify value) {
	justify_ = value;
	invalidate();
	return *this;
}

FlexLayout& FlexLayout::alignItems(Align value) {
	align_ = value;
	invalidate();
	return *this;
}

FlexLayout& FlexLayout::alignContent(AlignContent value) {
	align_content_ = value;
	invalidate();
	return *this;
}

FlexLayout& FlexLayout::gap(int value) {
	column_gap_ = value;
	row_gap_ = value;
	invalidate();
	return *this;
}

FlexLayout& FlexLayout::columnGap(int value) {
	column_gap_ = value;
	invalidate();
	return *this;
}

FlexLayout& FlexLayout::rowGap(int value) {
	row_gap_ = value;
	invalidate();
	return *this;
}

FlexLayout& FlexLayout::padding(int all) {
	padding_ = LayoutInsets::all(all);
	invalidate();
	return *this;
}

FlexLayout& FlexLayout::padding(LayoutInsets insets) {
	padding_ = insets;
	invalidate();
	return *this;
}

FlexItem* FlexLayout::find(const Window* child) const {
	for (const unique_ptr<FlexItem>& item : items_) {
		if (item->window_ == child) return item.get();
	}
	return nullptr;
}

FlexItem* FlexLayout::find(const LayoutNode* child) const {
	for (const unique_ptr<FlexItem>& item : items_) {
		if (item->node_ == child) return item.get();
	}
	return nullptr;
}

FlexItem& FlexLayout::add(Window& child) {
	if (FlexItem* existing = find(&child)) return *existing;
	items_.push_back(make_unique<FlexItem>(child, *this));
	invalidate();
	return *items_.back();
}

FlexItem& FlexLayout::add(LayoutNode& child) {
	if (&child == this) {
		throw std::invalid_argument("FlexLayout::add: a layout cannot contain itself");
	}
	if (FlexItem* existing = find(&child)) return *existing;
	items_.push_back(make_unique<FlexItem>(child, *this));
	invalidate();
	return *items_.back();
}

bool FlexLayout::remove(Window& child) {
	const auto it = std::find_if(items_.begin(), items_.end(),
		[&child](const unique_ptr<FlexItem>& item) { return item->window_ == &child; });
	if (it == items_.end()) return false;
	items_.erase(it);
	invalidate();
	return true;
}

bool FlexLayout::remove(LayoutNode& child) {
	const auto it = std::find_if(items_.begin(), items_.end(),
		[&child](const unique_ptr<FlexItem>& item) { return item->node_ == &child; });
	if (it == items_.end()) return false;
	items_.erase(it);
	invalidate();
	return true;
}

void FlexLayout::clear() {
	if (items_.empty()) return;
	items_.clear();
	invalidate();
}

detail::FlexSpec FlexLayout::build_spec() const {
	detail::FlexSpec spec;
	spec.direction = direction_;
	spec.wrap = wrap_;
	spec.justify = justify_;
	spec.align = align_;
	spec.alignContent = align_content_;
	spec.padding = padding_;
	spec.columnGap = column_gap_;
	spec.rowGap = row_gap_;
	return spec;
}

vector<detail::FlexItemSpec> FlexLayout::build_specs() const {
	vector<detail::FlexItemSpec> specs;
	specs.reserve(items_.size());
	for (const unique_ptr<FlexItem>& item : items_) {
		detail::FlexItemSpec spec = item->spec_;
		spec.result = LayoutRect{};
		if (item->window_) {
			spec.content = to_layout_size(item->window_->content_size());
			spec.contentMeasure = nullptr;
		}
		else if (item->node_) {
			LayoutNode* node = item->node_;
			spec.contentMeasure = [node](LayoutSize available) {
				return node->measure(available);
			};
		}
		specs.push_back(std::move(spec));
	}
	return specs;
}

LayoutSize FlexLayout::measure(LayoutSize available) const {
	vector<detail::FlexItemSpec> specs = build_specs();
	return solve_flex(build_spec(), specs, available).content;
}

vector<Placement> FlexLayout::arrange(const LayoutRect& area) {
	vector<Placement> placements;
	if (items_.empty()) {
		dirty_ = false;
		return placements;
	}

	const LayoutSize available{ (std::max)(0, area.width), (std::max)(0, area.height) };
	vector<detail::FlexItemSpec> specs = build_specs();
	(void)solve_flex(build_spec(), specs, available);

	for (size_t i = 0; i < items_.size(); ++i) {
		const FlexItem& item = *items_[i];
		const LayoutRect& local = specs[i].result;
		const LayoutRect absolute{
			area.x + local.x,
			area.y + local.y,
			local.width,
			local.height
		};

		if (item.window_) {
			Window& window = *item.window_;
			if (!window.created()) continue;
			const HWND hwnd = static_cast<HWND>(window);
			if (!hwnd || !IsWindow(hwnd)) continue;
			placements.push_back(Placement{ hwnd, absolute });
		}
		else if (item.node_) {
			vector<Placement> nested = item.node_->arrange(absolute);
			placements.insert(placements.end(), nested.begin(), nested.end());
		}
	}

	dirty_ = false;
	return placements;
}

void FlexLayout::invalidate() {
	if (dirty_) return;
	dirty_ = true;
	if (reflow_handler_) reflow_handler_();
}

void FlexLayout::set_reflow_handler(std::function<void()> handler) {
	reflow_handler_ = std::move(handler);
}
