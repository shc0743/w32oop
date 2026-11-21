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

// 布局基础类型。
// 这个头文件**不依赖任何 Windows 头文件**，因为布局求解器需要能脱离窗口单测。

#include <climits>
#include <cmath>

namespace w32oop::ui {

	// 逻辑像素尺寸。
	struct LayoutSize {
		int width = 0;
		int height = 0;

		// 「这一维不设限」的哨兵值，只在 measure/求解时使用。
		static constexpr int Unbounded = INT_MAX;

		[[nodiscard]] constexpr bool isWidthUnbounded() const noexcept { return width == Unbounded; }
		[[nodiscard]] constexpr bool isHeightUnbounded() const noexcept { return height == Unbounded; }
		[[nodiscard]] static constexpr LayoutSize unbounded() noexcept { return { Unbounded, Unbounded }; }

		friend constexpr bool operator==(const LayoutSize&, const LayoutSize&) noexcept = default;
	};

	// 逻辑像素矩形，左上角为原点。
	struct LayoutRect {
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;

		[[nodiscard]] constexpr int right() const noexcept { return x + width; }
		[[nodiscard]] constexpr int bottom() const noexcept { return y + height; }

		friend constexpr bool operator==(const LayoutRect&, const LayoutRect&) noexcept = default;
	};

	// 四边内/外边距，顺序与 CSS 的 padding/margin 简写一致：上、右、下、左。
	struct LayoutInsets {
		int top = 0;
		int right = 0;
		int bottom = 0;
		int left = 0;

		[[nodiscard]] static constexpr LayoutInsets all(int value) noexcept {
			return { value, value, value, value };
		}
		[[nodiscard]] static constexpr LayoutInsets symmetric(int horizontal, int vertical) noexcept {
			return { vertical, horizontal, vertical, horizontal };
		}

		[[nodiscard]] constexpr int horizontal() const noexcept { return left + right; }
		[[nodiscard]] constexpr int vertical() const noexcept { return top + bottom; }

		friend constexpr bool operator==(const LayoutInsets&, const LayoutInsets&) noexcept = default;
	};

	// 百分比，取值 0..100。必须显式构造，避免 width(50) 这种整数写法被误认成百分比。
	struct Percent {
		explicit constexpr Percent(float value) noexcept : value(value) {}
		float value = 0.0f;
	};

	// 长度：自动 / 绝对像素 / 百分比，三个状态互斥。
	class Length {
	public:
		enum class Unit { Auto, Pixels, Percent };

		constexpr Length() noexcept = default;
		constexpr Length(int pixels) noexcept : unit_(Unit::Pixels), value_(pixels) {}
		constexpr Length(Percent percent) noexcept : unit_(Unit::Percent), value_(percent.value) {}

		[[nodiscard]] constexpr Unit unit() const noexcept { return unit_; }
		[[nodiscard]] constexpr bool isAuto() const noexcept { return unit_ == Unit::Auto; }
		[[nodiscard]] constexpr bool isPixels() const noexcept { return unit_ == Unit::Pixels; }
		[[nodiscard]] constexpr bool isPercent() const noexcept { return unit_ == Unit::Percent; }

		[[nodiscard]] constexpr int pixels() const noexcept { return static_cast<int>(value_); }
		[[nodiscard]] constexpr float percent() const noexcept { return static_cast<float>(value_); }

		// 解析成逻辑像素。percentBase 为 Unbounded 时百分比退化为 fallback（与 CSS 一致）。
		[[nodiscard]] int resolve(int percentBase, int fallback) const noexcept {
			switch (unit_) {
			case Unit::Pixels:
				return static_cast<int>(std::lround(value_));
			case Unit::Percent:
				if (percentBase == LayoutSize::Unbounded) return fallback;
				return static_cast<int>(std::lround(static_cast<double>(percentBase) * value_ / 100.0));
			case Unit::Auto:
			default:
				return fallback;
			}
		}

		friend constexpr bool operator==(const Length&, const Length&) noexcept = default;

	private:
		Unit unit_ = Unit::Auto;
		double value_ = 0.0;
	};

	// 主轴方向。Row/Column 的反向版本只影响主轴坐标的镜像。
	enum class Direction { Row, RowReverse, Column, ColumnReverse };

	// 主轴剩余空间的分配方式，语义同 CSS justify-content。
	enum class Justify { Start, End, Center, SpaceBetween, SpaceAround, SpaceEvenly };

	// 交叉轴对齐，语义同 CSS align-items / align-self。
	// Auto 只用于 alignSelf：表示「继承容器的 alignItems」。
	enum class Align { Auto, Start, End, Center, Stretch };

	// 多行时行与行之间的分配方式，语义同 CSS align-content。
	enum class AlignContent { Start, End, Center, SpaceBetween, SpaceAround, SpaceEvenly };

	[[nodiscard]] constexpr bool is_horizontal(Direction direction) noexcept {
		return direction == Direction::Row || direction == Direction::RowReverse;
	}
	[[nodiscard]] constexpr bool is_reverse(Direction direction) noexcept {
		return direction == Direction::RowReverse || direction == Direction::ColumnReverse;
	}
}
