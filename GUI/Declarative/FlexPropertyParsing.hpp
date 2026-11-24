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
#include "./PropertyValue.hpp"
#include "../Layout/LayoutTypes.hpp"

#include <cmath>
#include <cstdlib>
#include <string>
#include <string_view>

namespace w32oop::ui::internal {

	struct ParsedLength {
		bool percent = false;
		int pixels = 0;
		float ratio = 0.0f;
	};

	[[nodiscard]] inline ParsedLength parse_length(const PropertyValue& value) {
		ParsedLength result;
		if (value.kind() == PropertyValue::Kind::Integer) {
			result.pixels = value.asInt();
			return result;
		}
		if (value.kind() == PropertyValue::Kind::Real) {
			result.pixels = static_cast<int>(std::lround(value.asReal()));
			return result;
		}
		const std::wstring text = value.asString();
		if (text.empty()) return result;
		if (text.back() == L'%') {
			result.percent = true;
			result.ratio = static_cast<float>(std::wcstod(text.c_str(), nullptr));
			return result;
		}
		result.pixels = std::wcstol(text.c_str(), nullptr, 10);
		return result;
	}

	[[nodiscard]] inline ui::LayoutInsets parse_insets(const PropertyValue& value) {
		if (value.kind() == PropertyValue::Kind::Insets) return value.asInsets();
		if (value.kind() == PropertyValue::Kind::Integer) {
			return ui::LayoutInsets::all(value.asInt());
		}
		if (const auto* style = value.asStyle()) {
			ui::LayoutInsets insets;
			for (const auto& entry : *style) {
				const int v = entry.second.asInt();
				if (entry.first.view() == L"top") insets.top = v;
				else if (entry.first.view() == L"right") insets.right = v;
				else if (entry.first.view() == L"bottom") insets.bottom = v;
				else if (entry.first.view() == L"left") insets.left = v;
			}
			return insets;
		}
		return {};
	}

	[[nodiscard]] inline bool parse_align(std::wstring_view name, bool& matched, ui::Align& out) {
		matched = true;
		if (name == L"start" || name == L"flex-start") out = ui::Align::Start;
		else if (name == L"end" || name == L"flex-end") out = ui::Align::End;
		else if (name == L"center") out = ui::Align::Center;
		else if (name == L"stretch") out = ui::Align::Stretch;
		else matched = false;
		return matched;
	}
}
