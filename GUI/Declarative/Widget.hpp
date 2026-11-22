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
#include "../Window.hpp"
#include "../FoundationLib.hpp"
#include "../Layout/FlexLayout.hpp"
#include "./Property.hpp"
#include "./VNode.hpp"
#include <ranges>
#include <span>
#include <type_traits>
#include <typeindex>


namespace w32oop::ui {

	class EventSink : public w32DeclarativeObject {
	public:
		virtual ~EventSink() = default;
		virtual void on(std::wstring_view event, EventHandler handler) = 0;
		virtual void emit(std::wstring_view event, const EventPayload& payload) = 0;
	};

	struct WidgetDescriptor {
		std::type_index type = std::type_index(typeid(void));
		bool is_container = false;

		std::unique_ptr<ui::Window> (*create)(ui::Window& parent) = nullptr;
		std::vector<PropertyDescriptor> (*properties)() = nullptr;
		void (*apply_text)(ui::Window&, const std::wstring&) = nullptr;
		bool (*apply_property)(ui::Window&, std::wstring_view, const PropertyValue&) = nullptr;
		void (*bind_events)(ui::Window&, EventSink&) = nullptr;
		bool (*set_ref)(ui::Window* window, const RefHandle& handle) = nullptr;
	};

	namespace internal {


		template<class T>
		bool set_text(ui::Window& window, const PropertyValue& value) {
			const std::wstring text = value.asString();
			if (window.text() == text) return true;
			window.text(text);
			return true;
		}

		template<class T>
		bool set_enabled(ui::Window& window, const PropertyValue& value) {
			window.enable(value.asBool());
			return true;
		}

		template<class T>
		bool set_visible(ui::Window& window, const PropertyValue& value) {
			if (value.asBool()) window.show();
			else window.hide();
			return true;
		}


		template<class T>
		struct StyleTraits {
			static constexpr bool supported = false;
			static bool apply(T&, std::wstring_view, const PropertyValue&) { return false; }
		};

		template<>
		struct StyleTraits<ui::foundation::StaticEx> {
			static constexpr bool supported = true;
			static bool apply(ui::foundation::StaticEx& control, std::wstring_view name, const PropertyValue& value) {
				if (name == L"color") {
					control.color(value.asColor());
					return true;
				}
				if (name == L"background-color") {
					control.backgroundColor(value.asColor());
					return true;
				}
				return false;
			}
		};

		template<class T>
		bool apply_style(ui::Window& window, const PropertyValue& value) {
			const PropertyValue::StyleMap* style = value.asStyle();
			if (!style) return false;
			if constexpr (!StyleTraits<T>::supported) {
				return false;
			}
			else {
				T& control = static_cast<T&>(window);
				bool ok = true;
				for (const auto& entry : *style) {
					if (!StyleTraits<T>::apply(control, entry.first.view(), entry.second)) ok = false;
				}
				return ok;
			}
		}


		template<class T>
		struct ExtraProperties {
			static void append(std::vector<PropertyDescriptor>&) {}
		};

		template<>
		struct ExtraProperties<ui::foundation::Edit> {
			static void append(std::vector<PropertyDescriptor>& out) {
				out.push_back(PropertyDescriptor{
					L"readonly",
					[](void* widget, const PropertyValue& value) {
						static_cast<ui::foundation::Edit*>(widget)->readonly(value.asBool());
						return true;
					}
				});
			}
		};

		template<class T>
		std::vector<PropertyDescriptor> build_properties() {
			std::vector<PropertyDescriptor> out;
			out.push_back(PropertyDescriptor{ L"value", [](void* widget, const PropertyValue& value) {
				return set_text<T>(*static_cast<ui::Window*>(widget), value);
			} });
			out.push_back(PropertyDescriptor{ L"text", [](void* widget, const PropertyValue& value) {
				return set_text<T>(*static_cast<ui::Window*>(widget), value);
			} });
			out.push_back(PropertyDescriptor{ L"enabled", [](void* widget, const PropertyValue& value) {
				return set_enabled<T>(*static_cast<ui::Window*>(widget), value);
			} });
			out.push_back(PropertyDescriptor{ L"visible", [](void* widget, const PropertyValue& value) {
				return set_visible<T>(*static_cast<ui::Window*>(widget), value);
			} });
			out.push_back(PropertyDescriptor{ L"style", [](void* widget, const PropertyValue& value) {
				return apply_style<T>(*static_cast<ui::Window*>(widget), value);
			} });
			ExtraProperties<T>::append(out);
			return out;
		}


		template<class T>
		struct EventTraits {
			static void bind(T&, EventSink&) {}
		};

		template<>
		struct EventTraits<ui::foundation::Edit> {
			static void bind(ui::foundation::Edit& control, EventSink& sink) {
				control.onChange([&control, &sink](ui::EventData&) {
					sink.emit(L"update:value", EventPayload{ control.text(), nullptr });
				});
			}
		};

		template<>
		struct EventTraits<ui::foundation::Button> {
			static void bind(ui::foundation::Button& control, EventSink& sink) {
				control.onClick([&sink](ui::EventData&) {
					sink.emit(L"click", EventPayload{});
				});
			}
		};

		template<>
		struct EventTraits<ui::foundation::CheckBox> {
			static void bind(ui::foundation::CheckBox& control, EventSink& sink) {
				control.onChanged([&control, &sink](ui::EventData&) {
					sink.emit(L"update:checked", EventPayload{ control.checked() ? L"1" : L"0", nullptr });
				});
			}
		};

		template<class T>
		concept ChildControl = std::derived_from<T, ui::foundation::ChildControlWindow>;
	}

	template<class T>
	struct WidgetTraits;

	template<internal::ChildControl T>
	struct WidgetTraits<T> {
		static constexpr bool adapted = true;

		static std::unique_ptr<T> create(ui::Window& parent) {
			auto control = std::make_unique<T>();
			control->set_parent(&parent);
			control->create(L"", 1, 1, 0, 0, 0, 0, nullptr);
			return control;
		}

		static const std::vector<PropertyDescriptor>& properties() {
			static const std::vector<PropertyDescriptor> table = internal::build_properties<T>();
			return table;
		}

		static void apply_text(T& control, const std::wstring& text) {
			if (control.text() != text) control.text(text);
		}

		static bool apply_property(T& control, std::wstring_view name, const PropertyValue& value) {
			const PropertyDescriptor* descriptor = find_property(properties(), name);
			if (!descriptor) return false;
			return descriptor->apply(&control, value);
		}

		static void bind_events(T& control, EventSink& sink) {
			internal::EventTraits<T>::bind(control, sink);
		}
	};

	template<class T>
	concept SelfDescribingWidget = std::derived_from<T, ui::Window>
		&& requires(T& control, ui::Window& parent, std::wstring_view name,
			const PropertyValue& value, EventSink& sink) {
			{ T::properties() } -> std::ranges::range;
			{ T::create(parent) } -> std::convertible_to<std::unique_ptr<T>>;
			{ control.apply_property(name, value) } -> std::same_as<bool>;
			{ control.bind_events(sink) };
			{ control.text(std::wstring{}) };
		};

	template<class T>
	concept AdaptedWidget = requires(ui::Window& parent, T& control, std::wstring_view name,
		const PropertyValue& value, EventSink& sink) {
		{ WidgetTraits<T>::adapted } -> std::convertible_to<bool>;
		{ WidgetTraits<T>::create(parent) } -> std::convertible_to<std::unique_ptr<T>>;
		{ WidgetTraits<T>::properties() } -> std::ranges::range;
		{ WidgetTraits<T>::apply_property(control, name, value) } -> std::same_as<bool>;
		{ WidgetTraits<T>::bind_events(control, sink) };
		{ WidgetTraits<T>::apply_text(control, std::wstring{}) };
	};

	template<class T>
	concept Widget = SelfDescribingWidget<T> || AdaptedWidget<T>;

	template<class T>
	concept LayoutContainer = std::derived_from<T, ui::LayoutNode> && std::default_initializable<T>;

	template<class T>
	concept HNode = Widget<T> || LayoutContainer<T>;

	namespace internal {

		template<class T>
		const WidgetDescriptor& widget_descriptor() {
			static const WidgetDescriptor descriptor = [] {
				WidgetDescriptor d;
				d.type = std::type_index(typeid(T));
				d.is_container = false;
				if constexpr (AdaptedWidget<T>) {
					d.create = [](ui::Window& parent) -> std::unique_ptr<ui::Window> {
						return WidgetTraits<T>::create(parent);
					};
					d.properties = []() -> std::vector<PropertyDescriptor> {
						return WidgetTraits<T>::properties();
					};
					d.apply_text = [](ui::Window& window, const std::wstring& text) {
						WidgetTraits<T>::apply_text(static_cast<T&>(window), text);
					};
					d.apply_property = [](ui::Window& window, std::wstring_view name, const PropertyValue& value) {
						return WidgetTraits<T>::apply_property(static_cast<T&>(window), name, value);
					};
					d.bind_events = [](ui::Window& window, EventSink& sink) {
						WidgetTraits<T>::bind_events(static_cast<T&>(window), sink);
					};
					d.set_ref = [](ui::Window* window, const RefHandle& handle) {
						void* pointer = window ? static_cast<void*>(&static_cast<T&>(*window)) : nullptr;
						return handle.set_pointer(typeid(T*), pointer);
					};
				}
				else {
					d.create = [](ui::Window& parent) -> std::unique_ptr<ui::Window> {
						return T::create(parent);
					};
					d.properties = []() -> std::vector<PropertyDescriptor> {
						const auto& table = T::properties();
						return std::vector<PropertyDescriptor>(table.begin(), table.end());
					};
					d.apply_text = [](ui::Window& window, const std::wstring& text) {
						if (window.text() != text) window.text(text);
					};
					d.apply_property = [](ui::Window& window, std::wstring_view name, const PropertyValue& value) {
						return static_cast<T&>(window).apply_property(name, value);
					};
					d.bind_events = [](ui::Window& window, EventSink& sink) {
						static_cast<T&>(window).bind_events(sink);
					};
					d.set_ref = [](ui::Window* window, const RefHandle& handle) {
						void* pointer = window ? static_cast<void*>(&static_cast<T&>(*window)) : nullptr;
						return handle.set_pointer(typeid(T*), pointer);
					};
				}
				return d;
			}();
			return descriptor;
		}

		template<class T>
		const WidgetDescriptor& container_descriptor() {
			static const WidgetDescriptor descriptor = [] {
				WidgetDescriptor d;
				d.type = std::type_index(typeid(T));
				d.is_container = true;
				return d;
			}();
			return descriptor;
		}

		template<HNode T>
		VNode make_node() {
			VNode node;
			node.type = std::type_index(typeid(T));
			if constexpr (LayoutContainer<T>) {
				node.widget = &container_descriptor<T>();
			}
			else {
				node.widget = &widget_descriptor<T>();
			}
			return node;
		}
	}

	using PropertyList = std::initializer_list<std::pair<PropertyKey, PropertyValue>>;

	template<HNode T>
	[[nodiscard]] VNode h(PropertyList props) {
		VNode node = internal::make_node<T>();
		node.props.assign(props.begin(), props.end());
		return node;
	}

	template<HNode T>
	[[nodiscard]] VNode h(PropertyList props, std::nullptr_t) {
		return h<T>(props);
	}

	template<HNode T>
	[[nodiscard]] VNode h(PropertyList props, std::initializer_list<VNode> children) {
		VNode node = h<T>(props);
		node.children.assign(children.begin(), children.end());
		return node;
	}

	template<HNode T>
	[[nodiscard]] VNode h(PropertyList props, const VNode& child) {
		VNode node = h<T>(props);
		node.children.push_back(child);
		return node;
	}

	template<HNode T>
	[[nodiscard]] VNode h(PropertyList props, const wchar_t* text) {
		VNode node = h<T>(props);
		node.children.push_back(VNode::text_node(std::wstring(text ? text : L"")));
		return node;
	}

	template<HNode T>
	[[nodiscard]] VNode h(PropertyList props, const std::wstring& text) {
		VNode node = h<T>(props);
		node.children.push_back(VNode::text_node(text));
		return node;
	}

	template<HNode T>
	[[nodiscard]] VNode h(PropertyList props, const Ref<std::wstring>& text) {
		VNode node = h<T>(props);
		node.children.push_back(VNode::text_node(text));
		return node;
	}

	[[nodiscard]] inline const WidgetDescriptor* descriptor_of(const VNode& node) noexcept {
		return static_cast<const WidgetDescriptor*>(node.widget);
	}
}
