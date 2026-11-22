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
#include "./LayoutTypes.hpp"


namespace w32oop::ui::internal {

	struct FlexItemSpec {
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

		LayoutSize content{};
		std::function<LayoutSize(LayoutSize available)> contentMeasure{};

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
		LayoutSize content{};
		int lineCount = 0;
	};

	[[nodiscard]] FlexResult solve_flex(
		const FlexSpec& spec,
		std::vector<FlexItemSpec>& items,
		LayoutSize available);
}
