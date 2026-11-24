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
#include "./Property.hpp"
#include "./FlexPropertyParsing.hpp"
#include "../Layout/FlexLayout.hpp"

#include <span>
#include <string_view>

namespace w32oop::ui::internal {

	inline bool set_direction(FlexLayout& box, const PropertyValue& value) {
		const std::wstring text = value.asString();
		if (text == L"row") box.direction(Direction::Row);
		else if (text == L"row-reverse") box.direction(Direction::RowReverse);
		else if (text == L"column") box.direction(Direction::Column);
		else if (text == L"column-reverse") box.direction(Direction::ColumnReverse);
		else return false;
		return true;
	}

	inline bool set_wrap(FlexLayout& box, const PropertyValue& value) {
		box.wrap(value.asBool());
		return true;
	}

	inline bool set_justify(FlexLayout& box, const PropertyValue& value) {
		const std::wstring text = value.asString();
		if (text == L"start" || text == L"flex-start") box.justifyContent(Justify::Start);
		else if (text == L"end" || text == L"flex-end") box.justifyContent(Justify::End);
		else if (text == L"center") box.justifyContent(Justify::Center);
		else if (text == L"space-between") box.justifyContent(Justify::SpaceBetween);
		else if (text == L"space-around") box.justifyContent(Justify::SpaceAround);
		else if (text == L"space-evenly") box.justifyContent(Justify::SpaceEvenly);
		else return false;
		return true;
	}

	inline bool set_align_items(FlexLayout& box, const PropertyValue& value) {
		Align align{};
		bool matched = false;
		(void)parse_align(value.asString(), matched, align);
		if (!matched) return false;
		box.alignItems(align);
		return true;
	}

	inline bool set_align_content(FlexLayout& box, const PropertyValue& value) {
		const std::wstring text = value.asString();
		if (text == L"start" || text == L"flex-start") box.alignContent(AlignContent::Start);
		else if (text == L"end" || text == L"flex-end") box.alignContent(AlignContent::End);
		else if (text == L"center") box.alignContent(AlignContent::Center);
		else if (text == L"space-between") box.alignContent(AlignContent::SpaceBetween);
		else if (text == L"space-around") box.alignContent(AlignContent::SpaceAround);
		else if (text == L"space-evenly") box.alignContent(AlignContent::SpaceEvenly);
		else return false;
		return true;
	}

	inline bool set_gap(FlexLayout& box, const PropertyValue& value) {
		box.gap(value.asInt());
		return true;
	}

	inline bool set_row_gap(FlexLayout& box, const PropertyValue& value) {
		box.rowGap(value.asInt());
		return true;
	}

	inline bool set_column_gap(FlexLayout& box, const PropertyValue& value) {
		box.columnGap(value.asInt());
		return true;
	}

	inline bool set_padding(FlexLayout& box, const PropertyValue& value) {
		box.padding(parse_insets(value));
		return true;
	}

	inline bool set_grow(FlexItem& item, const PropertyValue& value) {
		item.grow(static_cast<float>(value.asReal()));
		return true;
	}

	inline bool set_shrink(FlexItem& item, const PropertyValue& value) {
		item.shrink(static_cast<float>(value.asReal()));
		return true;
	}

	inline bool set_basis(FlexItem& item, const PropertyValue& value) {
		const ParsedLength length = parse_length(value);
		if (length.percent) item.basis(Percent{ length.ratio });
		else item.basis(length.pixels);
		return true;
	}

	inline bool set_width(FlexItem& item, const PropertyValue& value) {
		const ParsedLength length = parse_length(value);
		if (length.percent) item.width(Percent{ length.ratio });
		else item.width(length.pixels);
		return true;
	}

	inline bool set_height(FlexItem& item, const PropertyValue& value) {
		const ParsedLength length = parse_length(value);
		if (length.percent) item.height(Percent{ length.ratio });
		else item.height(length.pixels);
		return true;
	}

	inline bool set_min_width(FlexItem& item, const PropertyValue& value) {
		item.minWidth(parse_length(value).pixels);
		return true;
	}

	inline bool set_max_width(FlexItem& item, const PropertyValue& value) {
		item.maxWidth(parse_length(value).pixels);
		return true;
	}

	inline bool set_min_height(FlexItem& item, const PropertyValue& value) {
		item.minHeight(parse_length(value).pixels);
		return true;
	}

	inline bool set_max_height(FlexItem& item, const PropertyValue& value) {
		item.maxHeight(parse_length(value).pixels);
		return true;
	}

	inline bool set_margin(FlexItem& item, const PropertyValue& value) {
		item.margin(parse_insets(value));
		return true;
	}

	inline bool set_align_self(FlexItem& item, const PropertyValue& value) {
		Align align{};
		bool matched = false;
		(void)parse_align(value.asString(), matched, align);
		if (!matched) return false;
		item.alignSelf(align);
		return true;
	}

	inline bool apply_container(void* target, std::wstring_view name, const PropertyValue& value);

	inline bool apply_container_direction(void* t, const PropertyValue& v) { return set_direction(*static_cast<FlexLayout*>(t), v); }
	inline bool apply_container_wrap(void* t, const PropertyValue& v) { return set_wrap(*static_cast<FlexLayout*>(t), v); }
	inline bool apply_container_justify(void* t, const PropertyValue& v) { return set_justify(*static_cast<FlexLayout*>(t), v); }
	inline bool apply_container_align_items(void* t, const PropertyValue& v) { return set_align_items(*static_cast<FlexLayout*>(t), v); }
	inline bool apply_container_align_content(void* t, const PropertyValue& v) { return set_align_content(*static_cast<FlexLayout*>(t), v); }
	inline bool apply_container_gap(void* t, const PropertyValue& v) { return set_gap(*static_cast<FlexLayout*>(t), v); }
	inline bool apply_container_row_gap(void* t, const PropertyValue& v) { return set_row_gap(*static_cast<FlexLayout*>(t), v); }
	inline bool apply_container_column_gap(void* t, const PropertyValue& v) { return set_column_gap(*static_cast<FlexLayout*>(t), v); }
	inline bool apply_container_padding(void* t, const PropertyValue& v) { return set_padding(*static_cast<FlexLayout*>(t), v); }

	inline bool apply_item_grow(void* t, const PropertyValue& v) { return set_grow(*static_cast<FlexItem*>(t), v); }
	inline bool apply_item_shrink(void* t, const PropertyValue& v) { return set_shrink(*static_cast<FlexItem*>(t), v); }
	inline bool apply_item_basis(void* t, const PropertyValue& v) { return set_basis(*static_cast<FlexItem*>(t), v); }
	inline bool apply_item_width(void* t, const PropertyValue& v) { return set_width(*static_cast<FlexItem*>(t), v); }
	inline bool apply_item_height(void* t, const PropertyValue& v) { return set_height(*static_cast<FlexItem*>(t), v); }
	inline bool apply_item_min_width(void* t, const PropertyValue& v) { return set_min_width(*static_cast<FlexItem*>(t), v); }
	inline bool apply_item_max_width(void* t, const PropertyValue& v) { return set_max_width(*static_cast<FlexItem*>(t), v); }
	inline bool apply_item_min_height(void* t, const PropertyValue& v) { return set_min_height(*static_cast<FlexItem*>(t), v); }
	inline bool apply_item_max_height(void* t, const PropertyValue& v) { return set_max_height(*static_cast<FlexItem*>(t), v); }
	inline bool apply_item_margin(void* t, const PropertyValue& v) { return set_margin(*static_cast<FlexItem*>(t), v); }
	inline bool apply_item_align_self(void* t, const PropertyValue& v) { return set_align_self(*static_cast<FlexItem*>(t), v); }

	inline constexpr PropertyDescriptor kFlexContainerProperties[] = {
		{ L"flex-direction", &apply_container_direction },
		{ L"flex-wrap", &apply_container_wrap },
		{ L"justify-content", &apply_container_justify },
		{ L"align-items", &apply_container_align_items },
		{ L"align-content", &apply_container_align_content },
		{ L"gap", &apply_container_gap },
		{ L"row-gap", &apply_container_row_gap },
		{ L"column-gap", &apply_container_column_gap },
		{ L"padding", &apply_container_padding },
	};

	inline constexpr PropertyDescriptor kFlexItemProperties[] = {
		{ L"grow", &apply_item_grow },
		{ L"shrink", &apply_item_shrink },
		{ L"basis", &apply_item_basis },
		{ L"width", &apply_item_width },
		{ L"height", &apply_item_height },
		{ L"min-width", &apply_item_min_width },
		{ L"max-width", &apply_item_max_width },
		{ L"min-height", &apply_item_min_height },
		{ L"max-height", &apply_item_max_height },
		{ L"margin", &apply_item_margin },
		{ L"align-self", &apply_item_align_self },
	};
}

namespace w32oop::ui {

	[[nodiscard]] inline std::span<const PropertyDescriptor> flex_container_properties() {
		return internal::kFlexContainerProperties;
	}

	[[nodiscard]] inline std::span<const PropertyDescriptor> flex_item_properties() {
		return internal::kFlexItemProperties;
	}

	[[nodiscard]] inline bool is_layout_property(std::wstring_view name) {
		return find_property(flex_container_properties(), name) != nullptr
			|| find_property(flex_item_properties(), name) != nullptr;
	}

	inline bool apply_flex_container_property(FlexLayout& box, std::wstring_view name, const PropertyValue& value) {
		const PropertyDescriptor* descriptor = find_property(flex_container_properties(), name);
		return descriptor && descriptor->apply(&box, value);
	}

	inline bool apply_flex_item_property(FlexItem& item, std::wstring_view name, const PropertyValue& value) {
		const PropertyDescriptor* descriptor = find_property(flex_item_properties(), name);
		return descriptor && descriptor->apply(&item, value);
	}
}
