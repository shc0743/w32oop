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

// flexbox 布局容器。
//
// 用法：
//     FlexLayout root{Direction::Column};
//     root.padding(12).gap(8);
//     root.add(title).height(32);
//     root.add(editor).grow(1).basis(0).minHeight(0);
//
// 样式命名直接沿用 CSS 属性名（direction / wrap / justifyContent / alignItems /
// alignSelf / grow / shrink / basis / gap / padding / margin），
// 不再有 constraintXxx_toYyyOf 这类只有设计器生成器才写得出来的名字。

#include "./LayoutNode.hpp"
#include "./FlexSolver.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

namespace w32oop::ui {

	class Window;
	class FlexLayout;

	// 容器的子项。所有 setter 都返回 *this，可以链式书写；改动会请求一次重排。
	class FlexItem {
	public:
		FlexItem(Window& child, FlexLayout& owner) noexcept;
		FlexItem(LayoutNode& child, FlexLayout& owner) noexcept;

		FlexItem(const FlexItem&) = delete;
		FlexItem& operator=(const FlexItem&) = delete;

		// ---- 主轴 ----
		FlexItem& grow(float value);           // 剩余空间分配权重，默认 0
		FlexItem& shrink(float value);         // 空间不足时的收缩权重，默认 1
		FlexItem& basis(int pixels);           // flex-basis 固定值；basis(0) + grow(1) 即 CSS flex:1
		FlexItem& basis(Percent percent);

		// ---- 两轴尺寸 ----
		FlexItem& width(int pixels);
		FlexItem& width(Percent percent);
		FlexItem& height(int pixels);
		FlexItem& height(Percent percent);
		FlexItem& size(int width, int height);

		FlexItem& minWidth(int pixels);
		FlexItem& maxWidth(int pixels);
		FlexItem& minHeight(int pixels);
		FlexItem& maxHeight(int pixels);

		// ---- 盒模型与对齐 ----
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
		detail::FlexItemSpec spec_{};
	};

	// 一维 flexbox 容器。
	// 可以放进另一个 FlexLayout 里做嵌套布局；嵌套节点没有窗口，只是几何计算。
	class FlexLayout final : public LayoutNode {
	public:
		explicit FlexLayout(Direction direction = Direction::Row);
		~FlexLayout() override;

		FlexLayout(const FlexLayout&) = delete;
		FlexLayout& operator=(const FlexLayout&) = delete;

		// ---- 容器样式 ----
		FlexLayout& direction(Direction value);
		FlexLayout& wrap(bool value = true);
		FlexLayout& justifyContent(Justify value);
		FlexLayout& alignItems(Align value);
		FlexLayout& alignContent(AlignContent value);
		FlexLayout& gap(int value);            // 行列统一间距
		FlexLayout& columnGap(int value);
		FlexLayout& rowGap(int value);
		FlexLayout& padding(int all);
		FlexLayout& padding(LayoutInsets insets);

		// ---- 子项 ----
		FlexItem& add(Window& child);
		FlexItem& add(LayoutNode& child);
		bool remove(Window& child);
		bool remove(LayoutNode& child);
		void clear();
		[[nodiscard]] std::size_t item_count() const noexcept { return items_.size(); }

		// ---- LayoutNode ----
		[[nodiscard]] LayoutSize measure(LayoutSize available) const override;
		[[nodiscard]] std::vector<Placement> arrange(const LayoutRect& area) override;
		void invalidate() override;
		void set_reflow_handler(std::function<void()> handler) override;

	private:
		[[nodiscard]] FlexItem* find(const Window* child) const;
		[[nodiscard]] FlexItem* find(const LayoutNode* child) const;
		[[nodiscard]] std::vector<detail::FlexItemSpec> build_specs() const;
		[[nodiscard]] detail::FlexSpec build_spec() const;

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
