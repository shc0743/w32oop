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
#include <climits>
#include <cmath>

namespace w32oop::ui {
	struct LayoutSize {
		int width = 0;
		int height = 0;

		static constexpr int Unbounded = INT_MAX;

		[[nodiscard]] constexpr bool isWidthUnbounded() const noexcept { return width == Unbounded; }
		[[nodiscard]] constexpr bool isHeightUnbounded() const noexcept { return height == Unbounded; }
		[[nodiscard]] static constexpr LayoutSize unbounded() noexcept { return { Unbounded, Unbounded }; }

		friend constexpr bool operator==(const LayoutSize&, const LayoutSize&) noexcept = default;
	};

	struct LayoutRect {
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;

		[[nodiscard]] constexpr int right() const noexcept { return x + width; }
		[[nodiscard]] constexpr int bottom() const noexcept { return y + height; }

		friend constexpr bool operator==(const LayoutRect&, const LayoutRect&) noexcept = default;
	};

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

	struct Percent {
		explicit constexpr Percent(float value) noexcept : value(value) {}
		float value = 0.0f;
	};

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

	enum class Direction { Row, RowReverse, Column, ColumnReverse };
	enum class Justify { Start, End, Center, SpaceBetween, SpaceAround, SpaceEvenly };
	enum class Align { Auto, Start, End, Center, Stretch };
	enum class AlignContent { Start, End, Center, SpaceBetween, SpaceAround, SpaceEvenly };

	[[nodiscard]] constexpr bool is_horizontal(Direction direction) noexcept {
		return direction == Direction::Row || direction == Direction::RowReverse;
	}
	[[nodiscard]] constexpr bool is_reverse(Direction direction) noexcept {
		return direction == Direction::RowReverse || direction == Direction::ColumnReverse;
	}
}
