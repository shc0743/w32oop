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

#include "../def.hpp"
#include "./LayoutNode.hpp"
#include "./FlexSolver.hpp"


namespace w32oop::ui {

	class Window;
	class FlexLayout;

	class FlexItem : public w32LayoutManagerObject {
	public:
		FlexItem(Window& child, FlexLayout& owner) noexcept;
		FlexItem(LayoutNode& child, FlexLayout& owner) noexcept;

		FlexItem(const FlexItem&) = delete;
		FlexItem& operator=(const FlexItem&) = delete;

		FlexItem& grow(float value);
		FlexItem& shrink(float value);
		FlexItem& basis(int pixels);
		FlexItem& basis(Percent percent);

		FlexItem& width(int pixels);
		FlexItem& width(Percent percent);
		FlexItem& height(int pixels);
		FlexItem& height(Percent percent);
		FlexItem& size(int width, int height);

		FlexItem& minWidth(int pixels);
		FlexItem& maxWidth(int pixels);
		FlexItem& minHeight(int pixels);
		FlexItem& maxHeight(int pixels);

		FlexItem& margin(int all);
		FlexItem& margin(LayoutInsets insets);
		FlexItem& alignSelf(Align align);

		[[nodiscard]] Window* window() const noexcept { return window_; }
		[[nodiscard]] LayoutNode* node() const noexcept { return node_; }

	private:
		friend class FlexLayout;

		FlexLayout& owner_;
		Window* window_ = nullptr;
		LayoutNode* node_ = nullptr;
		internal::FlexItemSpec spec_{};
	};

	class FlexLayout final : public LayoutNode {
	public:
		explicit FlexLayout(Direction direction = Direction::Row);
		~FlexLayout() override;

		FlexLayout(const FlexLayout&) = delete;
		FlexLayout& operator=(const FlexLayout&) = delete;

		FlexLayout& direction(Direction value);
		FlexLayout& wrap(bool value = true);
		FlexLayout& justifyContent(Justify value);
		FlexLayout& alignItems(Align value);
		FlexLayout& alignContent(AlignContent value);
		FlexLayout& gap(int value);
		FlexLayout& columnGap(int value);
		FlexLayout& rowGap(int value);
		FlexLayout& padding(int all);
		FlexLayout& padding(LayoutInsets insets);

		FlexItem& add(Window& child);
		FlexItem& add(LayoutNode& child);
		bool remove(Window& child);
		bool remove(LayoutNode& child);
		void clear();
		[[nodiscard]] std::size_t item_count() const noexcept { return items_.size(); }

		[[nodiscard]] LayoutSize measure(LayoutSize available) const override;
		[[nodiscard]] std::vector<Placement> arrange(const LayoutRect& area) override;
		void invalidate() override;
		void set_reflow_handler(std::function<void()> handler) override;

	private:
		[[nodiscard]] FlexItem* find(const Window* child) const;
		[[nodiscard]] FlexItem* find(const LayoutNode* child) const;
		[[nodiscard]] std::vector<internal::FlexItemSpec> build_specs() const;
		[[nodiscard]] internal::FlexSpec build_spec() const;

		Direction direction_ = Direction::Row;
		bool wrap_ = false;
		Justify justify_ = Justify::Start;
		Align align_ = Align::Stretch;
		AlignContent align_content_ = AlignContent::Start;
		LayoutInsets padding_{};
		int column_gap_ = 0;
		int row_gap_ = 0;

		std::vector<std::unique_ptr<FlexItem>> items_;
		std::function<void()> reflow_handler_{};
		bool dirty_ = false;
	};
}
