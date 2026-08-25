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
#include <cstdint>
#include <string_view>
#include <stdexcept>


namespace w32oop::exceptions {
	w32oop_declare_exception_class_from(invalid_rgb_value, ui_exception);
}


namespace w32oop::ui {
	namespace internal::parse_rgb_color {
		constexpr bool is_space(char c) noexcept {
			return c == ' ' || c == '\t' || c == '\n'
				|| c == '\r' || c == '\f' || c == '\v';
		}

		constexpr bool is_digit(char c) noexcept {
			return c >= '0' && c <= '9';
		}

		constexpr bool is_hex_digit(char c) noexcept {
			return is_digit(c)
				|| (c >= 'a' && c <= 'f')
				|| (c >= 'A' && c <= 'F');
		}

		constexpr std::uint8_t hex_digit_value(char c) noexcept {
			if (is_digit(c)) return static_cast<std::uint8_t>(c - '0');
			if (c >= 'a' && c <= 'f') return static_cast<std::uint8_t>(c - 'a' + 10);
			return static_cast<std::uint8_t>(c - 'A' + 10);
		}

		constexpr std::size_t skip_spaces(std::string_view s, std::size_t pos) noexcept {
			while (pos < s.size() && is_space(s[pos])) ++pos;
			return pos;
		}

		constexpr COLORREF make_colorref(std::uint8_t r,
			std::uint8_t g,
			std::uint8_t b) noexcept {
			return RGB(r, g, b);
		}

		struct ComponentResult {
			std::uint8_t value;
			std::size_t  pos;
		};

		constexpr ComponentResult parse_component(std::string_view s, std::size_t pos) {
			pos = skip_spaces(s, pos);
			if (pos >= s.size())
				throw exceptions::invalid_rgb_value_exception{ "rgb(): unexpected end while parsing a component" };

			bool hex = false;
			if (pos + 1 < s.size() && s[pos] == '0'
				&& (s[pos + 1] == 'x' || s[pos + 1] == 'X')) {
				hex = true;
				pos += 2;
			}

			unsigned value = 0;
			std::size_t digits = 0;

			if (hex) {
				while (pos < s.size() && is_hex_digit(s[pos])) {
					value = value * 16u + hex_digit_value(s[pos]);
					++pos;
					++digits;
					if (value > 255u)
						throw exceptions::invalid_rgb_value_exception{ "rgb(): hex component out of range (0x00-0xFF)" };
				}
			}
			else {
				while (pos < s.size() && is_digit(s[pos])) {
					value = value * 10u + static_cast<unsigned>(s[pos] - '0');
					++pos;
					++digits;
					if (value > 255u)
						throw exceptions::invalid_rgb_value_exception{ "rgb(): decimal component out of range (0-255)" };
				}
			}

			if (digits == 0)
				throw exceptions::invalid_rgb_value_exception{ "rgb(): expected a number (decimal or 0x hex)" };

			return { static_cast<std::uint8_t>(value), pos };
		}

		constexpr bool starts_with_rgb_paren(std::string_view s) noexcept {
			return s.size() >= 4
				&& (s[0] == 'r' || s[0] == 'R')
				&& (s[1] == 'g' || s[1] == 'G')
				&& (s[2] == 'b' || s[2] == 'B')
				&& s[3] == '(';
		}

		constexpr COLORREF parse_rgb_function(std::string_view s) {
			std::size_t pos = 4; // skip "rgb("

			ComponentResult r = parse_component(s, pos);
			pos = r.pos;

			pos = skip_spaces(s, pos);
			if (pos >= s.size() || s[pos] != ',')
				throw exceptions::invalid_rgb_value_exception{ "rgb(): expected ',' after red component" };
			++pos;

			ComponentResult g = parse_component(s, pos);
			pos = g.pos;

			pos = skip_spaces(s, pos);
			if (pos >= s.size() || s[pos] != ',')
				throw exceptions::invalid_rgb_value_exception{ "rgb(): expected ',' after green component" };
			++pos;

			ComponentResult b = parse_component(s, pos);
			pos = b.pos;

			pos = skip_spaces(s, pos);
			if (pos >= s.size() || s[pos] != ')')
				throw exceptions::invalid_rgb_value_exception{ "rgb(): expected ')' after blue component" };
			++pos;

			pos = skip_spaces(s, pos);
			if (pos != s.size())
				throw exceptions::invalid_rgb_value_exception{ "rgb(): unexpected trailing characters after ')'" };

			return make_colorref(r.value, g.value, b.value);
		}

		constexpr COLORREF parse_hash_hex(std::string_view s) {
			const std::size_t len = s.size() - 1;

			if (len == 3) {
				for (std::size_t i = 1; i <= 3; ++i)
					if (!is_hex_digit(s[i]))
						throw exceptions::invalid_rgb_value_exception{ "#RGB: invalid hex digit" };

				std::uint8_t rv = hex_digit_value(s[1]);
				std::uint8_t gv = hex_digit_value(s[2]);
				std::uint8_t bv = hex_digit_value(s[3]);
				rv = static_cast<std::uint8_t>((rv << 4) | rv);
				gv = static_cast<std::uint8_t>((gv << 4) | gv);
				bv = static_cast<std::uint8_t>((bv << 4) | bv);
				return make_colorref(rv, gv, bv);
			}

			if (len == 6) {
				for (std::size_t i = 1; i <= 6; ++i)
					if (!is_hex_digit(s[i]))
						throw exceptions::invalid_rgb_value_exception{ "#RRGGBB: invalid hex digit" };

				const auto pair = [&](std::size_t i) noexcept -> std::uint8_t {
					return static_cast<std::uint8_t>(
						(hex_digit_value(s[i]) << 4) | hex_digit_value(s[i + 1]));
					};
				return make_colorref(pair(1), pair(3), pair(5));
			}

			throw exceptions::invalid_rgb_value_exception{ "hex color must be '#RGB' or '#RRGGBB'" };
		}

		constexpr COLORREF parse_impl(std::string_view input) {
			const std::size_t first = skip_spaces(input, 0);
			std::size_t last = input.size();
			while (last > first && is_space(input[last - 1])) --last;
			const std::string_view s = input.substr(first, last - first);

			if (s.empty())
				throw exceptions::invalid_rgb_value_exception{ "empty color string" };

			if (s[0] == '#')
				return parse_hash_hex(s);

			if (starts_with_rgb_paren(s))
				return parse_rgb_function(s);

			throw exceptions::invalid_rgb_value_exception{
				"unrecognized color format (use #RRGGBB, #RGB, or rgb(r,g,b))" };
		}

	}

	consteval COLORREF RgbColor(std::string_view str) {
		return internal::parse_rgb_color::parse_impl(str);
	}

	constexpr COLORREF ParseRgbColorRt(std::string_view str) {
		return internal::parse_rgb_color::parse_impl(str);
	}

	constexpr COLORREF RgbColor(std::uint32_t rgb) {
		if (rgb > 0xFFFFFFu)
			throw exceptions::invalid_rgb_value_exception{ "integer color must be equal or lower than 0xFFFFFF" };
		const auto r = static_cast<std::uint8_t>((rgb >> 16) & 0xFFu);
		const auto g = static_cast<std::uint8_t>((rgb >> 8) & 0xFFu);
		const auto b = static_cast<std::uint8_t>(rgb & 0xFFu);
		return internal::parse_rgb_color::make_colorref(r, g, b);
	}
}

