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
#include "../Layout/LayoutTypes.hpp"
#include "../../Reactive/Ref.hpp"
#include "../../Reactive/Computed.hpp"
#include <typeindex>


namespace w32oop::ui {
	class EventData;
}

namespace w32oop::ui {

	using w32oop::reactive::Observer;
	using w32oop::reactive::Observable;
	using w32oop::reactive::EffectScope;
	using w32oop::reactive::RefHandle;
	using w32oop::reactive::Ref;
	using w32oop::reactive::Computed;

	class PropertyKey : public w32DeclarativeObject {
	public:
		PropertyKey() = default;
		PropertyKey(const char* value) {
			if (value) text_.assign(value, value + std::strlen(value));
		}
		PropertyKey(const wchar_t* value) : text_(value ? value : L"") {}
		PropertyKey(std::wstring value) : text_(std::move(value)) {}

		[[nodiscard]] const std::wstring& str() const noexcept { return text_; }
		[[nodiscard]] std::wstring_view view() const noexcept { return text_; }

		bool operator<(const PropertyKey& other) const noexcept { return text_ < other.text_; }
		bool operator==(const PropertyKey& other) const noexcept { return text_ == other.text_; }

	private:
		std::wstring text_;
	};

	struct EventPayload {
		std::wstring text;
		ui::EventData* event = nullptr;
	};

	using EventHandler = std::function<void(const EventPayload&)>;

	struct RefBinding {
		RefHandle handle;
		std::type_index type = std::type_index(typeid(void));
	};

	class PropertyValue : public w32DeclarativeObject {
	public:
		enum class Kind {
			Null, Bool, Integer, Real, String, Color, Insets, Style, Handler, Ref
		};

		using StyleMap = std::map<PropertyKey, PropertyValue>;

		PropertyValue() = default;
		PropertyValue(std::nullptr_t) {}
		PropertyValue(bool value) : kind_(Kind::Bool), bool_(value) {}
		PropertyValue(int value) : kind_(Kind::Integer), integer_(value) {}
		PropertyValue(double value) : kind_(Kind::Real), real_(value) {}
		PropertyValue(const wchar_t* value) : kind_(Kind::String), string_(value ? value : L"") {}
		PropertyValue(const char* value) : kind_(Kind::String) {
			if (value) string_.assign(value, value + std::strlen(value));
		}
		PropertyValue(std::string value) : PropertyValue(value.c_str()) {}
		PropertyValue(std::wstring value) : kind_(Kind::String), string_(std::move(value)) {}
		PropertyValue(unsigned long value) : kind_(Kind::Color), integer_(static_cast<long long>(value)) {}
		PropertyValue(ui::LayoutInsets value) : kind_(Kind::Insets), insets_(value) {}

		PropertyValue(std::initializer_list<std::pair<PropertyKey, PropertyValue>> style)
			: kind_(Kind::Style) {
			style_ = std::make_shared<const StyleMap>(style.begin(), style.end());
		}

		template<class T>
		PropertyValue(Ref<T> ref);

		template<class T>
		PropertyValue(Computed<T> computed);

		template<class F>
			requires (std::is_invocable_v<F&, const EventPayload&>
				|| std::is_invocable_v<F&, std::wstring>
				|| std::is_invocable_v<F&>)
		PropertyValue(F&& handler)
			: kind_(Kind::Handler) {
			make_handler(std::forward<F>(handler));
		}

		[[nodiscard]] Kind kind() const noexcept { return kind_; }
		[[nodiscard]] bool isNull() const noexcept { return kind_ == Kind::Null; }

		[[nodiscard]] bool asBool(bool fallback = false) const noexcept {
			if (kind_ == Kind::Bool) return bool_;
			if (kind_ == Kind::Integer) return integer_ != 0;
			return fallback;
		}
		[[nodiscard]] int asInt(int fallback = 0) const noexcept {
			if (kind_ == Kind::Integer || kind_ == Kind::Color || kind_ == Kind::Bool) {
				return static_cast<int>(integer_);
			}
			return fallback;
		}
		[[nodiscard]] double asReal(double fallback = 0.0) const noexcept {
			if (kind_ == Kind::Real) return real_;
			if (kind_ == Kind::Integer) return static_cast<double>(integer_);
			return fallback;
		}
		[[nodiscard]] std::wstring asString(const std::wstring& fallback = L"") const {
			if (kind_ == Kind::String) return string_;
			return fallback;
		}
		[[nodiscard]] unsigned long asColor(unsigned long fallback = 0) const noexcept {
			if (kind_ == Kind::Color || kind_ == Kind::Integer) {
				return static_cast<unsigned long>(integer_);
			}
			return fallback;
		}
		[[nodiscard]] ui::LayoutInsets asInsets(ui::LayoutInsets fallback = {}) const noexcept {
			return kind_ == Kind::Insets ? insets_ : fallback;
		}
		[[nodiscard]] const StyleMap* asStyle() const noexcept {
			return style_ ? style_.get() : nullptr;
		}
		[[nodiscard]] const EventHandler* asHandler() const noexcept {
			return handler_ ? handler_.get() : nullptr;
		}
		[[nodiscard]] const RefBinding* asRef() const noexcept {
			return ref_ ? ref_.get() : nullptr;
		}

		[[nodiscard]] bool operator==(const PropertyValue& other) const {
			if (kind_ != other.kind_) return false;
			switch (kind_) {
			case Kind::Null: return true;
			case Kind::Bool: return bool_ == other.bool_;
			case Kind::Integer: return integer_ == other.integer_;
			case Kind::Real: return real_ == other.real_;
			case Kind::String: return string_ == other.string_;
			case Kind::Color: return integer_ == other.integer_;
			case Kind::Insets: return insets_ == other.insets_;
			case Kind::Style: {
				if (!style_ || !other.style_) return style_.get() == other.style_.get();
				return *style_ == *other.style_;
			}
			case Kind::Handler: return handler_.get() == other.handler_.get();
			case Kind::Ref:
				if (!ref_ || !other.ref_) return ref_.get() == other.ref_.get();
				return ref_->handle.shares_with(other.ref_->handle) && ref_->type == other.ref_->type;
			default: return false;
			}
		}

		[[nodiscard]] bool operator!=(const PropertyValue& other) const { return !(*this == other); }

	private:
		template<class F>
		void make_handler(F&& handler) {
			if constexpr (std::is_invocable_v<F&, const EventPayload&>) {
				handler_ = std::make_shared<EventHandler>(std::forward<F>(handler));
			}
			else if constexpr (std::is_invocable_v<F&, std::wstring>) {
				auto fn = std::forward<F>(handler);
				handler_ = std::make_shared<EventHandler>(
					[fn = std::move(fn)](const EventPayload& payload) mutable { fn(payload.text); });
			}
			else {
				auto fn = std::forward<F>(handler);
				handler_ = std::make_shared<EventHandler>(
					[fn = std::move(fn)](const EventPayload&) mutable { fn(); });
			}
		}

		Kind kind_ = Kind::Null;
		bool bool_ = false;
		long long integer_ = 0;
		double real_ = 0.0;
		std::wstring string_;
		ui::LayoutInsets insets_{};
		std::shared_ptr<const StyleMap> style_;
		std::shared_ptr<EventHandler> handler_;
		std::shared_ptr<RefBinding> ref_;
	};

	template<class T>
	[[nodiscard]] PropertyValue make_snapshot(const T& value) {
		if constexpr (std::is_same_v<T, bool>) return PropertyValue(value);
		else if constexpr (std::is_same_v<T, int>) return PropertyValue(value);
		else if constexpr (std::is_same_v<T, unsigned long>) return PropertyValue(value);
		else if constexpr (std::is_same_v<T, unsigned int>) return PropertyValue(static_cast<unsigned long>(value));
		else if constexpr (std::is_same_v<T, long long>) return PropertyValue(static_cast<int>(value));
		else if constexpr (std::is_same_v<T, double>) return PropertyValue(value);
		else if constexpr (std::is_same_v<T, float>) return PropertyValue(static_cast<double>(value));
		else if constexpr (std::is_same_v<T, std::wstring>) return PropertyValue(value);
		else if constexpr (std::is_convertible_v<T, std::wstring>) return PropertyValue(std::wstring(value));
		else return PropertyValue();
	}

	template<class T>
	PropertyValue::PropertyValue(Ref<T> ref)
		: PropertyValue(make_snapshot(ref.value())) {
		ref_ = std::make_shared<RefBinding>(RefBinding{ ref.handle(), std::type_index(typeid(T)) });
	}

	template<class T>
	PropertyValue::PropertyValue(Computed<T> computed)
		: PropertyValue(make_snapshot(computed.value())) {
	}
}
