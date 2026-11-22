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
#include "../Layout/FlexLayout.hpp"
#include "./PropertyValue.hpp"
#include <cmath>


// extremely slop file fuck you DeepSeek
namespace w32oop::ui {

	namespace internal {

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

		inline bool set_direction(ui::FlexLayout& box, const PropertyValue& value) {
			const std::wstring text = value.asString();
			if (text == L"row") box.direction(ui::Direction::Row);
			else if (text == L"row-reverse") box.direction(ui::Direction::RowReverse);
			else if (text == L"column") box.direction(ui::Direction::Column);
			else if (text == L"column-reverse") box.direction(ui::Direction::ColumnReverse);
			else return false;
			return true;
		}

		inline bool set_wrap(ui::FlexLayout& box, const PropertyValue& value) {
			box.wrap(value.asBool());
			return true;
		}

		inline bool set_justify(ui::FlexLayout& box, const PropertyValue& value) {
			const std::wstring text = value.asString();
			if (text == L"start" || text == L"flex-start") box.justifyContent(ui::Justify::Start);
			else if (text == L"end" || text == L"flex-end") box.justifyContent(ui::Justify::End);
			else if (text == L"center") box.justifyContent(ui::Justify::Center);
			else if (text == L"space-between") box.justifyContent(ui::Justify::SpaceBetween);
			else if (text == L"space-around") box.justifyContent(ui::Justify::SpaceAround);
			else if (text == L"space-evenly") box.justifyContent(ui::Justify::SpaceEvenly);
			else return false;
			return true;
		}

		inline bool set_align_items(ui::FlexLayout& box, const PropertyValue& value) {
			ui::Align align{};
			bool matched = false;
			(void)parse_align(value.asString(), matched, align);
			if (!matched) return false;
			box.alignItems(align);
			return true;
		}

		inline bool set_align_content(ui::FlexLayout& box, const PropertyValue& value) {
			const std::wstring text = value.asString();
			if (text == L"start" || text == L"flex-start") box.alignContent(ui::AlignContent::Start);
			else if (text == L"end" || text == L"flex-end") box.alignContent(ui::AlignContent::End);
			else if (text == L"center") box.alignContent(ui::AlignContent::Center);
			else if (text == L"space-between") box.alignContent(ui::AlignContent::SpaceBetween);
			else if (text == L"space-around") box.alignContent(ui::AlignContent::SpaceAround);
			else if (text == L"space-evenly") box.alignContent(ui::AlignContent::SpaceEvenly);
			else return false;
			return true;
		}

		inline bool set_gap(ui::FlexLayout& box, const PropertyValue& value) {
			box.gap(value.asInt());
			return true;
		}

		inline bool set_row_gap(ui::FlexLayout& box, const PropertyValue& value) {
			box.rowGap(value.asInt());
			return true;
		}

		inline bool set_column_gap(ui::FlexLayout& box, const PropertyValue& value) {
			box.columnGap(value.asInt());
			return true;
		}

		inline bool set_padding(ui::FlexLayout& box, const PropertyValue& value) {
			box.padding(parse_insets(value));
			return true;
		}

		inline bool set_grow(ui::FlexItem& item, const PropertyValue& value) {
			item.grow(static_cast<float>(value.asReal()));
			return true;
		}

		inline bool set_shrink(ui::FlexItem& item, const PropertyValue& value) {
			item.shrink(static_cast<float>(value.asReal()));
			return true;
		}

		inline bool set_basis(ui::FlexItem& item, const PropertyValue& value) {
			const ParsedLength length = parse_length(value);
			if (length.percent) item.basis(ui::Percent{ length.ratio });
			else item.basis(length.pixels);
			return true;
		}

		inline bool set_width(ui::FlexItem& item, const PropertyValue& value) {
			const ParsedLength length = parse_length(value);
			if (length.percent) item.width(ui::Percent{ length.ratio });
			else item.width(length.pixels);
			return true;
		}

		inline bool set_height(ui::FlexItem& item, const PropertyValue& value) {
			const ParsedLength length = parse_length(value);
			if (length.percent) item.height(ui::Percent{ length.ratio });
			else item.height(length.pixels);
			return true;
		}

		inline bool set_min_width(ui::FlexItem& item, const PropertyValue& value) {
			item.minWidth(parse_length(value).pixels);
			return true;
		}

		inline bool set_max_width(ui::FlexItem& item, const PropertyValue& value) {
			item.maxWidth(parse_length(value).pixels);
			return true;
		}

		inline bool set_min_height(ui::FlexItem& item, const PropertyValue& value) {
			item.minHeight(parse_length(value).pixels);
			return true;
		}

		inline bool set_max_height(ui::FlexItem& item, const PropertyValue& value) {
			item.maxHeight(parse_length(value).pixels);
			return true;
		}

		inline bool set_margin(ui::FlexItem& item, const PropertyValue& value) {
			item.margin(parse_insets(value));
			return true;
		}

		inline bool set_align_self(ui::FlexItem& item, const PropertyValue& value) {
			ui::Align align{};
			bool matched = false;
			(void)parse_align(value.asString(), matched, align);
			if (!matched) return false;
			item.alignSelf(align);
			return true;
		}

		struct LayoutProperty {
			std::wstring_view name;
			bool (*apply_container)(ui::FlexLayout&, const PropertyValue&);
			bool (*apply_item)(ui::FlexItem&, const PropertyValue&);
		};

		inline constexpr LayoutProperty kLayoutProperties[] = {
			{ L"flex-direction", &set_direction, nullptr },
			{ L"flex-wrap", &set_wrap, nullptr },
			{ L"justify-content", &set_justify, nullptr },
			{ L"align-items", &set_align_items, nullptr },
			{ L"align-content", &set_align_content, nullptr },
			{ L"gap", &set_gap, nullptr },
			{ L"row-gap", &set_row_gap, nullptr },
			{ L"column-gap", &set_column_gap, nullptr },
			{ L"padding", &set_padding, nullptr },

			{ L"grow", nullptr, &set_grow },
			{ L"shrink", nullptr, &set_shrink },
			{ L"basis", nullptr, &set_basis },
			{ L"width", nullptr, &set_width },
			{ L"height", nullptr, &set_height },
			{ L"min-width", nullptr, &set_min_width },
			{ L"max-width", nullptr, &set_max_width },
			{ L"min-height", nullptr, &set_min_height },
			{ L"max-height", nullptr, &set_max_height },
			{ L"margin", nullptr, &set_margin },
			{ L"align-self", nullptr, &set_align_self },
		};
	}

	[[nodiscard]] inline bool is_layout_property(std::wstring_view name) {
		for (const auto& property : internal::kLayoutProperties) {
			if (property.name == name) return true;
		}
		return false;
	}

	inline bool apply_layout_property(ui::FlexLayout& box, std::wstring_view name, const PropertyValue& value) {
		for (const auto& property : internal::kLayoutProperties) {
			if (property.name != name || !property.apply_container) continue;
			return property.apply_container(box, value);
		}
		return false;
	}

	inline bool apply_layout_property(ui::FlexItem& item, std::wstring_view name, const PropertyValue& value) {
		for (const auto& property : internal::kLayoutProperties) {
			if (property.name != name || !property.apply_item) continue;
			return property.apply_item(item, value);
		}
		return false;
	}
}
