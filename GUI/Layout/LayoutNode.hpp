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

namespace w32oop::ui {
	class Window;

	struct Placement {
		Window* window = nullptr;
		LayoutRect rect{};
	};

	class LayoutNode : public w32LayoutNodeObject {
	public:
		virtual ~LayoutNode() = default;

		[[nodiscard]] virtual LayoutSize measure(LayoutSize available) const = 0;
		[[nodiscard]] virtual std::vector<Placement> arrange(const LayoutRect& area) = 0;
		virtual void invalidate() = 0;
		virtual void set_reflow_handler(std::function<void()> handler) = 0;
	};
}
