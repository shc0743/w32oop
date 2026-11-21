#pragma once
/*
MIT License, Copyright (c) 2025 @chcs1013
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef __cplusplus
#error "Must be included in C++"
#endif

#include "./def.hpp"
#include "./Window.hpp"

#include <climits>
#include <cstddef>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <vector>


namespace w32oop::ui {
	class LayoutManager : public w32LayoutManagerObject {
	protected:
		Window* owner = nullptr;
		RECT bounds_{};          // area in the owner's client coordinate space
		bool nested_ = false;    // true once placed by a parent layout
		bool laying_out_ = false;
	public:
		LayoutManager() = default;
		LayoutManager(Window* ownerWindow): owner(ownerWindow) {}
		virtual ~LayoutManager() = default;

		void set_owner(Window* ownerWindow) noexcept { owner = ownerWindow; }

		// Content area available to this layout. A root layout returns the owner's
		// client rect; a nested layout returns the area assigned by its parent.
		[[nodiscard]] virtual RECT client_rect();

		// Called by a parent layout: place this layout in the parent's coordinate
		// space and immediately reflow its children.
		virtual void resize(int x, int y, int w, int h);

		// Current area occupied by this layout (all zero until placed by a parent).
		[[nodiscard]] RECT bounds() const noexcept { return bounds_; }
		[[nodiscard]] bool is_nested() const noexcept { return nested_; }

		// Re-layout. A root layout syncs the owner's client rect; a nested layout
		// uses the area assigned by its parent.
		void doLayout();

		[[nodiscard]] inline Window* owner_window() const { return owner; }

	protected:
		// Subclasses implement the actual layout algorithm here.
		virtual void layout_children() = 0;
	};

	// Non-owning handle to a layout target: a top-level Window or a nested LayoutManager.
	// An explicitly constructed nullptr means "parent content area" (constraint targets only).
	class LayoutRef {
	public:
		LayoutRef() noexcept = default;
		LayoutRef(std::nullptr_t) noexcept {}
		LayoutRef(Window& w) noexcept : window_(&w) {}
		LayoutRef(Window* w) noexcept : window_(w) {}
		LayoutRef(LayoutManager& l) noexcept : layout_(&l) {}
		LayoutRef(LayoutManager* l) noexcept : layout_(l) {}

		[[nodiscard]] bool valid() const noexcept {
			return window_ != nullptr || layout_ != nullptr;
		}
		[[nodiscard]] bool is_window() const noexcept { return window_ != nullptr; }
		[[nodiscard]] Window* window() const noexcept { return window_; }
		[[nodiscard]] LayoutManager* layout() const noexcept { return layout_; }

		[[nodiscard]] RECT intrinsic_rect() const;
		void resize(int x, int y, int w, int h) const;
		[[nodiscard]] bool created() const;
		[[nodiscard]] bool has_parent() const;
		void attach_to(Window& owner) const;

		[[nodiscard]] bool operator==(const LayoutRef& other) const noexcept {
			return window_ == other.window_ && layout_ == other.layout_;
		}
		[[nodiscard]] bool operator!=(const LayoutRef& other) const noexcept {
			return !(*this == other);
		}

		struct Hash {
			[[nodiscard]] std::size_t operator()(const LayoutRef& ref) const noexcept;
		};

	private:
		Window* window_ = nullptr;
		LayoutManager* layout_ = nullptr;
	};

	// pre defined layouts.

	class LinearLayout : public LayoutManager {
	public:
		static constexpr int MATCH_PARENT = -2;
		static constexpr int WRAP_CONTENT = -1;

		enum class Orientation { Vertical, Horizontal };
		enum class Gravity { Start, Center, End, Stretch };

		struct LinearParams {
			LayoutRef target;

			int width = WRAP_CONTENT;
			int height = WRAP_CONTENT;

			// Share of the remaining main-axis space. 0 means not participating.
			float weight = 0.0f;

			float marginTop = 0.0f, marginBottom = 0.0f, marginStart = 0.0f, marginEnd = 0.0f;

			// Cross-axis alignment of the item.
			Gravity gravity = Gravity::Start;

			bool intrinsicValid = false;
			int  intrinsicW = 0, intrinsicH = 0;

			int lastL = INT_MIN, lastT = INT_MIN, lastW = INT_MIN, lastH = INT_MIN;
		};

		class Builder {
		private:
			LinearLayout* layout_;
			LinearParams* p_;
		public:
			Builder(LinearLayout* layout, LinearParams* params) noexcept
				: layout_(layout), p_(params) {
			}
			Builder(const Builder&) = default;
			Builder& operator=(const Builder&) = default;

			[[nodiscard]] LinearParams& raw() const noexcept { return *p_; }
			[[nodiscard]] LayoutRef target() const noexcept { return p_->target; }
			[[nodiscard]] LinearLayout& layout() const noexcept { return *layout_; }

			Builder& width(int v) noexcept { p_->width = v; return *this; }
			Builder& height(int v) noexcept { p_->height = v; return *this; }
			Builder& size(int w, int h) noexcept { p_->width = w; p_->height = h; return *this; }

			Builder& matchParent() noexcept { p_->width = p_->height = MATCH_PARENT; return *this; }
			Builder& wrapContent() noexcept { p_->width = p_->height = WRAP_CONTENT; return *this; }

			Builder& weight(float v) noexcept { p_->weight = v; return *this; }

			Builder& marginTop(float v) noexcept { p_->marginTop = v; return *this; }
			Builder& marginBottom(float v) noexcept { p_->marginBottom = v; return *this; }
			Builder& marginStart(float v) noexcept { p_->marginStart = v; return *this; }
			Builder& marginEnd(float v) noexcept { p_->marginEnd = v; return *this; }
			Builder& marginLeft(float v) noexcept { p_->marginStart = v; return *this; }
			Builder& marginRight(float v) noexcept { p_->marginEnd = v; return *this; }
			Builder& margin(float v) noexcept {
				p_->marginTop = p_->marginBottom = p_->marginStart = p_->marginEnd = v;
				return *this;
			}

			Builder& gravity(Gravity g) noexcept { p_->gravity = g; return *this; }
			Builder& gravityStart() noexcept { p_->gravity = Gravity::Start; return *this; }
			Builder& gravityCenter() noexcept { p_->gravity = Gravity::Center; return *this; }
			Builder& gravityEnd() noexcept { p_->gravity = Gravity::End; return *this; }
			Builder& gravityStretch() noexcept { p_->gravity = Gravity::Stretch; return *this; }
		};

	public:
		LinearLayout() = default;
		LinearLayout(Window* ownerWindow, Orientation orientation = Orientation::Vertical)
			: LayoutManager(ownerWindow), orientation_(orientation) {
		}
		virtual ~LinearLayout() = default;

		[[nodiscard]] Builder define(LayoutRef target);

		void remove(LayoutRef target);
		void clear_layout();

		void refresh_intrinsic(LayoutRef target);
		void refresh_intrinsic();

		[[nodiscard]] LinearParams* params_of(LayoutRef target);
		[[nodiscard]] const LinearParams* params_of(LayoutRef target) const;

		void orientation(Orientation o) noexcept { orientation_ = o; }
		[[nodiscard]] Orientation orientation() const noexcept { return orientation_; }

	protected:
		std::unordered_map<LayoutRef, LinearParams, LayoutRef::Hash> params;
		std::vector<LayoutRef> order;
		Orientation orientation_ = Orientation::Vertical;

		void register_node(LayoutRef target);
		void collect_intrinsics();
		void validate() const;
		void layout_children() override;
	};

	class ConstraintLayout : public LayoutManager {
	public:
		static constexpr int MATCH_PARENT     = -2;
		static constexpr int WRAP_CONTENT     = -1;
		static constexpr int MATCH_CONSTRAINT =  0;

		enum class Edge { Left, Top, Right, Bottom, Start, End };

		struct Link {
			LayoutRef target;
			Edge      targetEdge = Edge::Top;
			bool      set        = false;
		};

		struct ConstraintParams {
			LayoutRef target;

			int width  = WRAP_CONTENT;
			int height = WRAP_CONTENT;

			Link top, bottom, start, end;

			float marginTop = 0.0f, marginBottom = 0.0f, marginStart = 0.0f, marginEnd = 0.0f;

			float biasH = 0.5f, biasV = 0.5f;
			bool  hasBiasH = false, hasBiasV = false;

			float ratio = 0.0f;

			bool intrinsicValid = false;
			int  intrinsicW = 0, intrinsicH = 0;

			int lastL = INT_MIN, lastT = INT_MIN, lastW = INT_MIN, lastH = INT_MIN;
		};

		class Builder {
		private:
			ConstraintLayout* layout_;
			ConstraintParams* p_;
		public:
			Builder(ConstraintLayout* layout, ConstraintParams* params) noexcept
				: layout_(layout), p_(params) {
			}
			Builder(const Builder&) = default;
			Builder& operator=(const Builder&) = default;

			[[nodiscard]] ConstraintParams& raw() const noexcept { return *p_; }
			[[nodiscard]] LayoutRef target() const noexcept { return p_->target; }
			[[nodiscard]] ConstraintLayout& layout() const noexcept { return *layout_; }

			Builder& width(int v) noexcept { p_->width = v; return *this; }
			Builder& height(int v) noexcept { p_->height = v; return *this; }
			Builder& size(int w, int h) noexcept { p_->width = w; p_->height = h; return *this; }

			Builder& matchParent() noexcept { p_->width = p_->height = MATCH_PARENT; return *this; }
			Builder& matchConstraint() noexcept { p_->width = p_->height = MATCH_CONSTRAINT; return *this; }
			Builder& wrapContent() noexcept { p_->width = p_->height = WRAP_CONTENT; return *this; }

			Builder& constraintTop_toTopOf(LayoutRef anchor) noexcept { p_->top = { anchor, Edge::Top, true }; return *this; }
			Builder& constraintTop_toBottomOf(LayoutRef anchor) noexcept { p_->top = { anchor, Edge::Bottom, true }; return *this; }

			Builder& constraintBottom_toTopOf(LayoutRef anchor) noexcept { p_->bottom = { anchor, Edge::Top, true }; return *this; }
			Builder& constraintBottom_toBottomOf(LayoutRef anchor) noexcept { p_->bottom = { anchor, Edge::Bottom, true }; return *this; }

			Builder& constraintStart_toStartOf(LayoutRef anchor) noexcept { p_->start = { anchor, Edge::Start, true }; return *this; }
			Builder& constraintStart_toEndOf(LayoutRef anchor) noexcept { p_->start = { anchor, Edge::End, true }; return *this; }

			Builder& constraintEnd_toEndOf(LayoutRef anchor) noexcept { p_->end = { anchor, Edge::End, true }; return *this; }
			Builder& constraintEnd_toStartOf(LayoutRef anchor) noexcept { p_->end = { anchor, Edge::Start, true }; return *this; }

			Builder& constraintLeft_toLeftOf(LayoutRef anchor) noexcept { return constraintStart_toStartOf(anchor); }
			Builder& constraintLeft_toRightOf(LayoutRef anchor) noexcept { return constraintStart_toEndOf(anchor); }
			Builder& constraintRight_toRightOf(LayoutRef anchor) noexcept { return constraintEnd_toEndOf(anchor); }
			Builder& constraintRight_toLeftOf(LayoutRef anchor) noexcept { return constraintEnd_toStartOf(anchor); }

			Builder& marginTop(float v) noexcept { p_->marginTop = v; return *this; }
			Builder& marginBottom(float v) noexcept { p_->marginBottom = v; return *this; }
			Builder& marginStart(float v) noexcept { p_->marginStart = v; return *this; }
			Builder& marginEnd(float v) noexcept { p_->marginEnd = v; return *this; }
			Builder& marginLeft(float v) noexcept { p_->marginStart = v; return *this; }
			Builder& marginRight(float v) noexcept { p_->marginEnd = v; return *this; }
			Builder& margin(float v) noexcept {
				p_->marginTop = p_->marginBottom = p_->marginStart = p_->marginEnd = v;
				return *this;
			}

			Builder& horizontalBias(float bias) noexcept { p_->biasH = bias; p_->hasBiasH = true; return *this; }
			Builder& verticalBias(float bias) noexcept { p_->biasV = bias; p_->hasBiasV = true; return *this; }
			Builder& dimensionRatio(float widthOverHeight) noexcept { p_->ratio = widthOverHeight; return *this; }

			Builder& centerHorizontally() noexcept {
				p_->start = { LayoutRef{}, Edge::Start, true };
				p_->end = { LayoutRef{}, Edge::End, true };
				return *this;
			}
			Builder& centerVertically() noexcept {
				p_->top = { LayoutRef{}, Edge::Top, true };
				p_->bottom = { LayoutRef{}, Edge::Bottom, true };
				return *this;
			}
			Builder& centerInParent() noexcept { return centerHorizontally().centerVertically(); }

			Builder& clearHorizontal() noexcept { p_->start = Link{}; p_->end = Link{}; return *this; }
			Builder& clearVertical() noexcept { p_->top = Link{}; p_->bottom = Link{}; return *this; }
		};

	public:
		ConstraintLayout() = default;
		ConstraintLayout(Window* ownerWindow): LayoutManager(ownerWindow) {}
		virtual ~ConstraintLayout() = default;

		[[nodiscard]] Builder define(LayoutRef target);

		void remove(LayoutRef target);
		void clear_layout();

		void refresh_intrinsic(LayoutRef target);
		void refresh_intrinsic();

		[[nodiscard]] ConstraintParams* params_of(LayoutRef target);
		[[nodiscard]] const ConstraintParams* params_of(LayoutRef target) const;

	protected:
		std::unordered_map<LayoutRef, ConstraintParams, LayoutRef::Hash> params;
		std::vector<LayoutRef> order;

		void register_node(LayoutRef target);
		void collect_intrinsics();
		void validate() const;
		void layout_children() override;
	};
}
