#include "LayoutManager.hpp"
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <stdexcept>
#include <string>

using namespace w32oop;
using namespace w32oop::util;
using namespace w32oop::ui;

namespace {
	struct ResolvedRect {
		float l = 0.0f, t = 0.0f, w = 0.0f, h = 0.0f;
	};

	inline bool layout_debug_mode() {
		return Window::get_global_option(Window::Option_DebugMode) != 0;
	}

	void layout_vwarn(const wchar_t* manager, const wchar_t* format, va_list args) {
		va_list copy;
		va_copy(copy, args);
		int len = _vscwprintf(format, copy);
		va_end(copy);
		if (len <= 0) return;

		std::wstring text(static_cast<size_t>(len), L'\0');
		vswprintf_s(text.data(), text.size() + 1, format, args);

		std::wstring line = L"[w32oop::ui::";
		line += manager;
		line += L"]: ";
		line += text;
		OutputDebugStringW(line.c_str());
	}

	void layout_warn(const wchar_t* format, ...) {
		va_list args;
		va_start(args, format);
		layout_vwarn(L"ConstraintLayout", format, args);
		va_end(args);
	}

	void linear_warn(const wchar_t* format, ...) {
		va_list args;
		va_start(args, format);
		layout_vwarn(L"LinearLayout", format, args);
		va_end(args);
	}

	std::wstring node_name(const LayoutRef& node) {
		if (Window* w = node.window()) {
			std::wstring name = w->get_class_name();
			return name.empty() ? L"<window>" : name;
		}
		return node.layout() ? L"<layout>" : L"<parent>";
	}

	void* node_id(const LayoutRef& node) {
		return node.window()
			? static_cast<void*>(node.window())
			: static_cast<void*>(node.layout());
	}
}

// ---------------------------------------------------------------------------
// LayoutManager
// ---------------------------------------------------------------------------

RECT LayoutManager::client_rect() {
	if (nested_ || !owner) return bounds_;
	return owner->client_rect();
}

void LayoutManager::resize(int x, int y, int w, int h) {
	if (w < 0) w = 0;
	if (h < 0) h = 0;

	bounds_ = RECT{ x, y, x + w, y + h };
	nested_ = true;
	doLayout();
}

void LayoutManager::doLayout() {
	if (!nested_) {
		if (!owner) return;
		bounds_ = owner->client_rect();
	}
	if (laying_out_) return;  // stop recursion when nested layouts reference each other

	laying_out_ = true;
	layout_children();
	laying_out_ = false;
}

SIZE LayoutManager::measure(int availableW, int availableH) {
	if (availableW < 0) availableW = 0;
	if (availableH < 0) availableH = 0;

	const RECT saved_bounds = bounds_;
	const bool saved_nested = nested_;
	const bool saved_laying = laying_out_;
	const bool saved_measuring = measuring_;

	bounds_ = RECT{ 0, 0, availableW, availableH };
	nested_ = true;
	laying_out_ = true;
	measuring_ = true;
	try {
		layout_children();
	}
	catch (...) {
		bounds_ = saved_bounds;
		nested_ = saved_nested;
		laying_out_ = saved_laying;
		measuring_ = saved_measuring;
		throw;
	}
	bounds_ = saved_bounds;
	nested_ = saved_nested;
	laying_out_ = saved_laying;
	measuring_ = saved_measuring;

	SIZE result = last_content_;
	if (result.cx <= 0) result.cx = (LONG)availableW;
	if (result.cy <= 0) result.cy = (LONG)availableH;
	return result;
}

// ---------------------------------------------------------------------------
// LayoutRef
// ---------------------------------------------------------------------------

RECT LayoutRef::intrinsic_rect() const {
	if (window_) {
		if (!window_->created()) return RECT{};
		return window_->rect();
	}
	if (layout_) return layout_->bounds();
	return RECT{};
}

void LayoutRef::resize(int x, int y, int w, int h) const {
	if (window_) {
		window_->resize(x, y, w, h);
		return;
	}
	if (layout_) layout_->resize(x, y, w, h);
}

bool LayoutRef::created() const {
	if (window_) return window_->created();
	return layout_ != nullptr;
}

bool LayoutRef::has_parent() const {
	if (window_) return window_->has_parent();
	return layout_ != nullptr;
}

void LayoutRef::attach_to(Window& owner) const {
	if (window_) owner.append(*window_);
}

std::size_t LayoutRef::Hash::operator()(const LayoutRef& ref) const noexcept {
	const size_t h1 = std::hash<const void*>{}(ref.window());
	const size_t h2 = std::hash<const void*>{}(ref.layout());
	return h1 ^ (h2 + static_cast<size_t>(0x9e3779b97f4a7c15ULL) + (h1 << 6) + (h1 >> 2));
}

// ---------------------------------------------------------------------------
// ConstraintLayout
// ---------------------------------------------------------------------------

void ConstraintLayout::register_node(LayoutRef target) {
	if (!target.valid() || target.layout() == this) return;
	ConstraintParams& p = params[target];
	p.target = target;
	if (std::find(order.begin(), order.end(), target) == order.end()) {
		order.push_back(target);
	}
}

ConstraintLayout::Builder ConstraintLayout::define(LayoutRef target) {
	if (!target.valid() || target.layout() == this) {
		throw std::invalid_argument("ConstraintLayout::define: invalid or self target");
	}
	register_node(target);
	return Builder(this, &params.at(target));
}

void ConstraintLayout::remove(LayoutRef target) {
	params.erase(target);
	order.erase(std::remove(order.begin(), order.end(), target), order.end());
}

void ConstraintLayout::clear_layout() {
	params.clear();
	order.clear();
}

void ConstraintLayout::refresh_intrinsic(LayoutRef target) {
	if (ConstraintParams* p = params_of(target)) p->intrinsicValid = false;
}

void ConstraintLayout::refresh_intrinsic() {
	for (auto& entry : params) entry.second.intrinsicValid = false;
}

ConstraintLayout::ConstraintParams* ConstraintLayout::params_of(LayoutRef target) {
	auto it = params.find(target);
	return it == params.end() ? nullptr : &it->second;
}

const ConstraintLayout::ConstraintParams* ConstraintLayout::params_of(LayoutRef target) const {
	auto it = params.find(target);
	return it == params.end() ? nullptr : &it->second;
}

void ConstraintLayout::validate() const {
	if (!layout_debug_mode()) return;

	for (const LayoutRef& node : order) {
		auto it = params.find(node);
		if (it == params.end()) continue;
		const ConstraintParams& p = it->second;
		const std::wstring name = node_name(node);
		const wchar_t* n = name.c_str();
		auto W = [&](const wchar_t* msg) { layout_warn(L"%ls (%p): %ls", n, node_id(node), msg); };

		const bool hs = p.start.set, he = p.end.set, ht = p.top.set, hb = p.bottom.set;

		if (p.width == MATCH_CONSTRAINT && !(hs && he))  W(L"width=MATCH_CONSTRAINT requires start and end");
		if (p.height == MATCH_CONSTRAINT && !(ht && hb)) W(L"height=MATCH_CONSTRAINT requires top and bottom");
		if (p.width == MATCH_PARENT && (hs || he))       W(L"width=MATCH_PARENT ignores start/end");
		if (p.height == MATCH_PARENT && (ht || hb))      W(L"height=MATCH_PARENT ignores top/bottom");
		if (!hs && !he)                                  W(L"no horizontal constraint, x=0");
		if (!ht && !hb)                                  W(L"no vertical constraint, y=0");
		if (p.marginStart != 0.0f && !hs)                W(L"marginStart ignored: no start constraint");
		if (p.marginEnd != 0.0f && !he)                  W(L"marginEnd ignored: no end constraint");
		if (p.marginTop != 0.0f && !ht)                  W(L"marginTop ignored: no top constraint");
		if (p.marginBottom != 0.0f && !hb)               W(L"marginBottom ignored: no bottom constraint");
		if (p.biasH < 0.0f || p.biasH > 1.0f)            W(L"horizontalBias out of [0,1], clamped");
		if (p.biasV < 0.0f || p.biasV > 1.0f)            W(L"verticalBias out of [0,1], clamped");
		if (p.ratio < 0.0f)                              W(L"dimensionRatio must be > 0, ignored");
		if (p.ratio > 0.0f && p.height > 0)              W(L"dimensionRatio overrides explicit height");
		if (p.ratio > 0.0f && p.height == MATCH_PARENT)  W(L"dimensionRatio overrides height=MATCH_PARENT");
	}
}

void ConstraintLayout::collect_intrinsics() {
	for (const LayoutRef& node : order) {
		ConstraintParams& p = params[node];
		if (node.layout()) {
			// A nested layout's size is decided by its parent; reread it each pass to avoid stale caches.
			const RECT rc = node.intrinsic_rect();
			p.intrinsicW = (std::max)(0, (int)(rc.right - rc.left));
			p.intrinsicH = (std::max)(0, (int)(rc.bottom - rc.top));
			p.intrinsicValid = true;
			continue;
		}
		if (p.intrinsicValid) continue;
		const RECT rc = node.intrinsic_rect();
		p.intrinsicW = (std::max)(0, (int)(rc.right - rc.left));
		p.intrinsicH = (std::max)(0, (int)(rc.bottom - rc.top));
		p.intrinsicValid = true;
	}
}

void ConstraintLayout::layout_children() {
	if (order.empty()) return;

	{
		std::vector<LayoutRef> missing;
		for (auto& entry : params) {
			const ConstraintParams& p = entry.second;
			const Link* links[4] = { &p.top, &p.bottom, &p.start, &p.end };
			for (const Link* link : links) {
				if (!link->set || !link->target.valid()) continue;
				if (!params.contains(link->target)) missing.push_back(link->target);
			}
		}
		for (const LayoutRef& node : missing) register_node(node);
	}

	if (owner && owner->created()) {
		for (const LayoutRef& node : order) {
			if (!node.is_window()) continue;
			if (!node.created() || node.has_parent()) continue;
			node.attach_to(*owner);
		}
	}

	validate();

	collect_intrinsics();

	const RECT client = client_rect();
	const float PX = (float)client.left;
	const float PY = (float)client.top;
	const float PW = (float)(client.right - client.left);
	const float PH = (float)(client.bottom - client.top);

	std::unordered_map<LayoutRef, ResolvedRect, LayoutRef::Hash> result;
	result.reserve(order.size() * 2);
	for (const LayoutRef& node : order) {
		const ConstraintParams& p = params[node];
		ResolvedRect r;
		r.w = (p.width > 0) ? (float)p.width : (float)p.intrinsicW;
		r.h = (p.height > 0) ? (float)p.height : (float)p.intrinsicH;
		result.emplace(node, r);
	}

	auto edge_of = [&](const LayoutRef& target, Edge e) -> float {
		if (!target.valid()) {
			switch (e) {
			case Edge::Left: case Edge::Start:  return PX;
			case Edge::Right: case Edge::End:   return PX + PW;
			case Edge::Top:                     return PY;
			case Edge::Bottom:                  return PY + PH;
			}
			return 0.0f;
		}
		auto it = result.find(target);
		if (it == result.end()) return 0.0f;
		const ResolvedRect& r = it->second;
		switch (e) {
		case Edge::Left: case Edge::Start:  return r.l;
		case Edge::Right: case Edge::End:   return r.l + r.w;
		case Edge::Top:                     return r.t;
		case Edge::Bottom:                  return r.t + r.h;
		}
		return 0.0f;
	};

	const int max_pass = (int)order.size() + 4;
	for (int pass = 0; pass < max_pass; ++pass) {
		bool changed = false;

		for (const LayoutRef& node : order) {
			const ConstraintParams& p = params[node];
			ResolvedRect& r = result[node];
			const ResolvedRect old = r;

			if (p.width > 0) {
				r.w = (float)p.width;
			}
			else if (p.width == MATCH_PARENT || (p.width == WRAP_CONTENT && node.layout())) {
				r.w = (std::max)(0.0f, PW - p.marginStart - p.marginEnd);
			}
			else if (p.width == MATCH_CONSTRAINT && p.start.set && p.end.set) {
				const float a = edge_of(p.start.target, p.start.targetEdge) + p.marginStart;
				const float b = edge_of(p.end.target, p.end.targetEdge) - p.marginEnd;
				r.w = (std::max)(0.0f, b - a);
			}
			else {
				r.w = (float)p.intrinsicW;
			}

			bool height_locked = false;
			if (p.ratio > 0.0f && r.w > 0.0f) {
				r.h = r.w / p.ratio;
				height_locked = true;
			}
			if (!height_locked) {
				if (p.height > 0) {
					r.h = (float)p.height;
				}
				else if (p.height == MATCH_PARENT || (p.height == WRAP_CONTENT && node.layout())) {
					r.h = (std::max)(0.0f, PH - p.marginTop - p.marginBottom);
				}
				else if (p.height == MATCH_CONSTRAINT && p.top.set && p.bottom.set) {
					const float a = edge_of(p.top.target, p.top.targetEdge) + p.marginTop;
					const float b = edge_of(p.bottom.target, p.bottom.targetEdge) - p.marginBottom;
					r.h = (std::max)(0.0f, b - a);
				}
				else {
					r.h = (float)p.intrinsicH;
				}
			}

			if (p.start.set && p.end.set) {
				const float a = edge_of(p.start.target, p.start.targetEdge);
				const float b = edge_of(p.end.target, p.end.targetEdge);
				const float free_space = b - a - p.marginEnd - p.marginStart - r.w;
				r.l = a + p.marginStart + p.biasH * free_space;
			}
			else if (p.start.set) {
				r.l = edge_of(p.start.target, p.start.targetEdge) + p.marginStart;
			}
			else if (p.end.set) {
				r.l = edge_of(p.end.target, p.end.targetEdge) - p.marginEnd - r.w;
			}
			else {
				r.l = PX;
			}

			if (p.top.set && p.bottom.set) {
				const float a = edge_of(p.top.target, p.top.targetEdge);
				const float b = edge_of(p.bottom.target, p.bottom.targetEdge);
				const float free_space = b - a - p.marginBottom - p.marginTop - r.h;
				r.t = a + p.marginTop + p.biasV * free_space;
			}
			else if (p.top.set) {
				r.t = edge_of(p.top.target, p.top.targetEdge) + p.marginTop;
			}
			else if (p.bottom.set) {
				r.t = edge_of(p.bottom.target, p.bottom.targetEdge) - p.marginBottom - r.h;
			}
			else {
				r.t = PY;
			}

			if (old.l != r.l || old.t != r.t || old.w != r.w || old.h != r.h) changed = true;
		}

		if (!changed) break;
	}

	float minL = 0.0f, minT = 0.0f, maxR = 0.0f, maxB = 0.0f;
	for (const LayoutRef& node : order) {
		const ResolvedRect& r = result[node];
		const float l = r.l - PX;
		const float t = r.t - PY;
		minL = (std::min)(minL, l);
		minT = (std::min)(minT, t);
		maxR = (std::max)(maxR, l + r.w);
		maxB = (std::max)(maxB, t + r.h);
	}
	last_content_ = SIZE{
		(LONG)std::lround((std::max)(0.0f, maxR - minL)),
		(LONG)std::lround((std::max)(0.0f, maxB - minT))
	};

	if (layout_debug_mode()) {
		for (const LayoutRef& node : order) {
			const ConstraintParams& p = params[node];
			const ResolvedRect& r = result[node];
			const std::wstring name = node_name(node);
			const wchar_t* n = name.c_str();
			void* id = node_id(node);

			if (p.start.set && p.end.set) {
				const float gap = edge_of(p.end.target, p.end.targetEdge)
					- edge_of(p.start.target, p.start.targetEdge) - p.marginEnd - p.marginStart - r.w;
				if (gap < -0.5f) layout_warn(L"%ls (%p): horizontal conflict, gap %g", n, id, gap);
			}
			if (p.top.set && p.bottom.set) {
				const float gap = edge_of(p.bottom.target, p.bottom.targetEdge)
					- edge_of(p.top.target, p.top.targetEdge) - p.marginBottom - p.marginTop - r.h;
				if (gap < -0.5f) layout_warn(L"%ls (%p): vertical conflict, gap %g", n, id, gap);
			}
			if (r.w <= 0.0f || r.h <= 0.0f) {
				layout_warn(L"%ls (%p): resolved size %gx%g, not visible", n, id, r.w, r.h);
			}
		}
	}

	for (const LayoutRef& node : order) {
		if (node.is_window() && !node.created()) continue;

		ConstraintParams& p = params[node];
		const ResolvedRect& r = result[node];

		int left = (int)std::lround(r.l);
		int top = (int)std::lround(r.t);
		int width = (int)std::lround(r.w);
		int height = (int)std::lround(r.h);
		if (width < 0) width = 0;
		if (height < 0) height = 0;

		// Nested layouts must be driven every pass; they decide internally whether to reflow their children.
		if (node.is_window()
			&& left == p.lastL && top == p.lastT && width == p.lastW && height == p.lastH) {
			continue;
		}

		p.lastL = left;
		p.lastT = top;
		p.lastW = width;
		p.lastH = height;
		try {
			node.resize(left, top, width, height);
		}
		catch (const exceptions::window_not_initialized_exception&) {
			p.lastL = INT_MIN;
		}
	}
}

// ---------------------------------------------------------------------------
// LinearLayout
// ---------------------------------------------------------------------------

void LinearLayout::register_node(LayoutRef target) {
	if (!target.valid() || target.layout() == this) return;
	LinearParams& p = params[target];
	p.target = target;
	if (std::find(order.begin(), order.end(), target) == order.end()) {
		order.push_back(target);
	}
}

LinearLayout::Builder LinearLayout::define(LayoutRef target) {
	if (!target.valid() || target.layout() == this) {
		throw std::invalid_argument("LinearLayout::define: invalid or self target");
	}
	register_node(target);
	return Builder(this, &params.at(target));
}

void LinearLayout::remove(LayoutRef target) {
	params.erase(target);
	order.erase(std::remove(order.begin(), order.end(), target), order.end());
}

void LinearLayout::clear_layout() {
	params.clear();
	order.clear();
}

void LinearLayout::refresh_intrinsic(LayoutRef target) {
	if (LinearParams* p = params_of(target)) p->intrinsicValid = false;
}

void LinearLayout::refresh_intrinsic() {
	for (auto& entry : params) entry.second.intrinsicValid = false;
}

LinearLayout::LinearParams* LinearLayout::params_of(LayoutRef target) {
	auto it = params.find(target);
	return it == params.end() ? nullptr : &it->second;
}

const LinearLayout::LinearParams* LinearLayout::params_of(LayoutRef target) const {
	auto it = params.find(target);
	return it == params.end() ? nullptr : &it->second;
}

void LinearLayout::validate() const {
	if (!layout_debug_mode()) return;

	for (const LayoutRef& node : order) {
		auto it = params.find(node);
		if (it == params.end()) continue;
		const LinearParams& p = it->second;
		const std::wstring name = node_name(node);
		const wchar_t* n = name.c_str();
		auto W = [&](const wchar_t* msg) { linear_warn(L"%ls (%p): %ls", n, node_id(node), msg); };

		if (p.width < MATCH_PARENT)  W(L"width below MATCH_PARENT, treated as WRAP_CONTENT");
		if (p.height < MATCH_PARENT) W(L"height below MATCH_PARENT, treated as WRAP_CONTENT");
		if (p.weight < 0.0f)         W(L"negative weight ignored");
		if (p.width != MATCH_PARENT && p.width != WRAP_CONTENT && p.width < 0) {
			W(L"width must be 0, positive, MATCH_PARENT or WRAP_CONTENT");
		}
		if (p.height != MATCH_PARENT && p.height != WRAP_CONTENT && p.height < 0) {
			W(L"height must be 0, positive, MATCH_PARENT or WRAP_CONTENT");
		}
	}
}

void LinearLayout::collect_intrinsics() {
	for (const LayoutRef& node : order) {
		LinearParams& p = params[node];
		if (node.layout()) {
			// A nested layout's size is decided by its parent; reread it each pass.
			const RECT rc = node.intrinsic_rect();
			p.intrinsicW = (std::max)(0, (int)(rc.right - rc.left));
			p.intrinsicH = (std::max)(0, (int)(rc.bottom - rc.top));
			p.intrinsicValid = true;
			continue;
		}
		if (p.intrinsicValid) continue;
		const RECT rc = node.intrinsic_rect();
		p.intrinsicW = (std::max)(0, (int)(rc.right - rc.left));
		p.intrinsicH = (std::max)(0, (int)(rc.bottom - rc.top));
		p.intrinsicValid = true;
	}
}

void LinearLayout::layout_children() {
	if (order.empty()) return;

	if (owner && owner->created()) {
		for (const LayoutRef& node : order) {
			if (!node.is_window()) continue;
			if (!node.created() || node.has_parent()) continue;
			node.attach_to(*owner);
		}
	}

	validate();
	collect_intrinsics();

	const RECT client = client_rect();
	const float PX = (float)client.left;
	const float PY = (float)client.top;
	const float PW = (float)(client.right - client.left);
	const float PH = (float)(client.bottom - client.top);

	const bool vertical = orientation_ == Orientation::Vertical;
	const float mainStart  = vertical ? PY : PX;
	const float mainSpace  = vertical ? PH : PW;
	const float crossStart = vertical ? PX : PY;
	const float crossSpace = vertical ? PW : PH;

	struct Resolved {
		float l = 0.0f, t = 0.0f, w = 0.0f, h = 0.0f;
		float main = 0.0f;
		float cross = 0.0f;
	};

	std::unordered_map<LayoutRef, Resolved, LayoutRef::Hash> result;
	result.reserve(order.size() * 2);

	float fixedMain = 0.0f;
	float totalMargins = 0.0f;
	float totalWeight = 0.0f;

	for (const LayoutRef& node : order) {
		const LinearParams& p = params[node];
		Resolved& r = result[node];

		const float mStart = vertical ? p.marginTop : p.marginStart;
		const float mEnd   = vertical ? p.marginBottom : p.marginEnd;
		const float cStart = vertical ? p.marginStart : p.marginTop;
		const float cEnd   = vertical ? p.marginEnd : p.marginBottom;
		const int   mainSpec = vertical ? p.height : p.width;
		const int   crossSpec = vertical ? p.width : p.height;
		const int   intrinsicMain = vertical ? p.intrinsicH : p.intrinsicW;
		const int   intrinsicCross = vertical ? p.intrinsicW : p.intrinsicH;

		const float weight = p.weight > 0.0f ? p.weight : 0.0f;
		const float availMain = (std::max)(0.0f, mainSpace - mStart - mEnd);
		const float availCross = (std::max)(0.0f, crossSpace - cStart - cEnd);

		// A nested layout computes its own natural size when it is WRAP_CONTENT,
		// or when this LinearLayout itself is being measured (AT_MOST) on the cross axis.
		SIZE natural{};
		const bool measureNatural = node.layout() != nullptr
			&& (mainSpec == WRAP_CONTENT || crossSpec == WRAP_CONTENT
				|| (crossSpec == MATCH_PARENT && measuring_));
		if (measureNatural) {
			const int availableW = (int)(vertical ? availCross : availMain);
			const int availableH = (int)(vertical ? availMain : availCross);
			natural = node.layout()->measure(availableW, availableH);
		}

		float base = 0.0f;
		if (mainSpec > 0) {
			base = (float)mainSpec;
		}
		else if (mainSpec == 0) {
			// 0dp: only a weight can give it space.
			base = 0.0f;
		}
		else if (mainSpec == MATCH_PARENT) {
			base = (weight > 0.0f) ? 0.0f : availMain;
		}
		else { // WRAP_CONTENT
			base = measureNatural
				? (float)(vertical ? natural.cy : natural.cx)
				: (float)intrinsicMain;
			if (base > availMain) base = availMain;
		}

		float cross = 0.0f;
		if (crossSpec > 0) {
			cross = (float)crossSpec;
		}
		else if (crossSpec == 0) {
			cross = 0.0f;
		}
		else if (crossSpec == MATCH_PARENT) {
			if (measuring_) {
				// This LinearLayout is WRAP_CONTENT on the cross axis: match_parent
				// degrades to wrap_content.
				cross = measureNatural
					? (float)(vertical ? natural.cx : natural.cy)
					: (float)intrinsicCross;
			}
			else {
				cross = availCross;
			}
		}
		else { // WRAP_CONTENT
			cross = measureNatural
				? (float)(vertical ? natural.cx : natural.cy)
				: (float)intrinsicCross;
		}
		if (p.gravity == Gravity::Stretch) cross = availCross;
		if (availCross > 0.0f && cross > availCross) cross = availCross;

		r.main = base;
		r.cross = cross;
		totalMargins += mStart + mEnd;
		if (weight > 0.0f) {
			totalWeight += weight;
		}
		else {
			fixedMain += base;
		}
	}

	const float remaining = mainSpace - totalMargins - fixedMain;

	if (totalWeight > 0.0f) {
		for (const LayoutRef& node : order) {
			const LinearParams& p = params[node];
			const float weight = p.weight > 0.0f ? p.weight : 0.0f;
			if (weight <= 0.0f) continue;
			if (remaining > 0.0f) result[node].main += remaining * weight / totalWeight;
		}
	}

	float usedMain = totalMargins;
	for (const LayoutRef& node : order) usedMain += result[node].main;

	float offset = 0.0f;
	if (content_gravity_ == Gravity::Center) offset = (mainSpace - usedMain) * 0.5f;
	else if (content_gravity_ == Gravity::End) offset = mainSpace - usedMain;

	float cursor = mainStart + offset;
	for (const LayoutRef& node : order) {
		const LinearParams& p = params[node];
		Resolved& r = result[node];

		const float mStart = vertical ? p.marginTop : p.marginStart;
		const float mEnd   = vertical ? p.marginBottom : p.marginEnd;
		const float cStart = vertical ? p.marginStart : p.marginTop;
		const float cEnd   = vertical ? p.marginEnd : p.marginBottom;
		const float cross = r.cross;

		float crossPos = crossStart + cStart;
		switch (p.gravity) {
		case Gravity::Center:
			crossPos = crossStart + (crossSpace - cross) * 0.5f;
			break;
		case Gravity::End:
			crossPos = crossStart + crossSpace - cEnd - cross;
			break;
		case Gravity::Start:
		case Gravity::Stretch:
		default:
			crossPos = crossStart + cStart;
			break;
		}
		if (crossPos < crossStart + cStart && cross <= crossSpace - cStart - cEnd) {
			crossPos = crossStart + cStart;
		}

		const float mainPos = cursor + mStart;

		if (vertical) {
			r.l = crossPos;
			r.t = mainPos;
			r.w = cross;
			r.h = r.main;
		}
		else {
			r.t = crossPos;
			r.l = mainPos;
			r.w = r.main;
			r.h = cross;
		}

		cursor += mStart + r.main + mEnd;
	}

	float minCross = 0.0f, maxCross = 0.0f;
	for (const LayoutRef& node : order) {
		const Resolved& rr = result[node];
		const float crossPos = vertical ? rr.l : rr.t;
		const float crossSize = vertical ? rr.w : rr.h;
		const float rel = crossPos - crossStart;
		minCross = (std::min)(minCross, rel);
		maxCross = (std::max)(maxCross, rel + crossSize);
	}
	const float contentCross = (std::max)(0.0f, maxCross - minCross);
	last_content_ = vertical
		? SIZE{ (LONG)std::lround(contentCross), (LONG)std::lround(usedMain) }
		: SIZE{ (LONG)std::lround(usedMain), (LONG)std::lround(contentCross) };

	if (layout_debug_mode()) {
		if (cursor - mainStart > mainSpace + 0.5f) {
			linear_warn(L"content overflows main axis: need %g, available %g",
				cursor - mainStart, mainSpace);
		}
		for (const LayoutRef& node : order) {
			const Resolved& r = result[node];
			if (r.w <= 0.0f || r.h <= 0.0f) {
				const std::wstring name = node_name(node);
				linear_warn(L"%ls (%p): resolved size %gx%g, not visible",
					name.c_str(), node_id(node), r.w, r.h);
			}
		}
	}

	for (const LayoutRef& node : order) {
		if (node.is_window() && !node.created()) continue;

		LinearParams& p = params[node];
		const Resolved& r = result[node];

		int left = (int)std::lround(r.l);
		int top = (int)std::lround(r.t);
		int width = (int)std::lround(r.w);
		int height = (int)std::lround(r.h);
		if (width < 0) width = 0;
		if (height < 0) height = 0;

		// Nested layouts must be driven every pass; they decide internally whether to reflow their children.
		if (node.is_window()
			&& left == p.lastL && top == p.lastT && width == p.lastW && height == p.lastH) {
			continue;
		}

		p.lastL = left;
		p.lastT = top;
		p.lastW = width;
		p.lastH = height;
		try {
			node.resize(left, top, width, height);
		}
		catch (const exceptions::window_not_initialized_exception&) {
			p.lastL = INT_MIN;
		}
	}
}
