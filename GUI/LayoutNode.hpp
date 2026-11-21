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
#include "./LayoutTypes.hpp"

#include <cstddef>
#include <functional>
#include <vector>

namespace w32oop::ui {

	// 一次布局产生的一个窗口落位。坐标相对布局根窗口的客户区（逻辑像素）。
	struct Placement {
		HWND hwnd = nullptr;
		LayoutRect rect{};
	};

	// 布局节点的**纯接口**：没有数据成员，没有实现细节，只描述「布局需要什么」。
	// 具体状态（子项、样式、脏标记）由实现类（如 FlexLayout）自己持有。
	class LayoutNode {
	public:
		virtual ~LayoutNode() = default;

		// 在 available 空间下，这个节点期望占多大（包含它自己的 padding）。
		// 某一维取 LayoutSize::Unbounded 表示该方向不设限。
		[[nodiscard]] virtual LayoutSize measure(LayoutSize available) const = 0;

		// 在 area 内排布自己，返回整棵子树中所有窗口的落位。
		// area 的坐标是相对布局根窗口客户区的逻辑像素。
		[[nodiscard]] virtual std::vector<Placement> arrange(const LayoutRect& area) = 0;

		// 内容或结构变化了，请求宿主稍后重排。实现内部应做去重。
		virtual void invalidate() = 0;

		// 宿主（Window）注册重排回调；实现保存这个回调。
		virtual void set_reflow_handler(std::function<void()> handler) = 0;
	};
}
