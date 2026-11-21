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

// flexbox 求解器：纯函数，不接触 HWND / Window，因此可以脱离窗口做单元测试。
// 用法见 GUI/FlexLayout.cpp，测试见 .test/layout/flex_solver_tests.cpp。

#include "./LayoutTypes.hpp"

#include <climits>
#include <functional>
#include <vector>

namespace w32oop::ui::detail {

	// 一个子项交给求解器的全部信息。
	// 适配层（FlexLayout）负责把它填满，求解器只做几何计算。
	struct FlexItemSpec {
		// 主轴基准尺寸。Auto 时依次回退到 width/height、内容尺寸。
		Length basis{};
		Length width{};
		Length height{};

		int minWidth = 0;
		int minHeight = 0;
		int maxWidth = INT_MAX;
		int maxHeight = INT_MAX;

		float grow = 0.0f;
		float shrink = 1.0f;

		LayoutInsets margin{};
		Align alignSelf = Align::Auto;

		// 内容尺寸。嵌套布局可以设置 contentMeasure 来按可用空间现算。
		LayoutSize content{};
		std::function<LayoutSize(LayoutSize available)> contentMeasure{};

		// 输出：相对容器左上角（含 padding 偏移）的矩形。
		LayoutRect result{};
	};

	struct FlexSpec {
		Direction direction = Direction::Row;
		bool wrap = false;
		Justify justify = Justify::Start;
		Align align = Align::Stretch;
		AlignContent alignContent = AlignContent::Start;
		LayoutInsets padding{};
		int columnGap = 0;
		int rowGap = 0;
	};

	struct FlexResult {
		// 内容包围盒（含 padding），即 measure 时该返回的期望尺寸。
		LayoutSize content{};
		int lineCount = 0;
	};

	// 按 CSS flexbox 的子集排布 items。会写入 items[i].result。
	// available 的某一维可以是 LayoutSize::Unbounded，表示该方向不设限。
	[[nodiscard]] FlexResult solve_flex(
		const FlexSpec& spec,
		std::vector<FlexItemSpec>& items,
		LayoutSize available);
}
