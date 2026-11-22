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
#include "./Effect.hpp"
#include <typeindex>
#include <typeinfo>


namespace w32oop::reactive {
	class RefStateBase : public Observable {
	public:
		~RefStateBase() override = default;

		[[nodiscard]] virtual std::type_index value_type() const noexcept = 0;

		virtual bool write_text(const std::wstring&) { return false; }
		[[nodiscard]] virtual bool read_text(std::wstring&) const { return false; }
		virtual bool write_pointer(const std::type_info&, void*) { return false; }
		[[nodiscard]] virtual bool read_pointer(const std::type_info&, void*&) const { return false; }
	};

	class RefHandle : public w32ReactiveObject {
	public:
		RefHandle() = default;

		[[nodiscard]] bool valid() const noexcept { return state_ != nullptr; }
		[[nodiscard]] std::type_index value_type() const noexcept {
			return state_ ? state_->value_type() : std::type_index(typeid(void));
		}

		[[nodiscard]] bool set_text(const std::wstring& value) const {
			return state_ && state_->write_text(value);
		}
		[[nodiscard]] bool get_text(std::wstring& value) const {
			return state_ && state_->read_text(value);
		}
		[[nodiscard]] bool set_pointer(const std::type_info& type, void* pointer) const {
			return state_ && state_->write_pointer(type, pointer);
		}
		[[nodiscard]] bool get_pointer(const std::type_info& type, void*& pointer) const {
			return state_ && state_->read_pointer(type, pointer);
		}
		[[nodiscard]] bool shares_with(const RefHandle& other) const noexcept {
			return state_ == other.state_;
		}

	private:
		template<class>
		friend class Ref;

		explicit RefHandle(std::shared_ptr<RefStateBase> state) noexcept : state_(std::move(state)) {}

		std::shared_ptr<RefStateBase> state_;
	};

	template<class T>
	struct RefState final : RefStateBase {
		T value{};

		[[nodiscard]] std::type_index value_type() const noexcept override { return typeid(T); }
	};

	template<>
	struct RefState<std::wstring> final : RefStateBase {
		std::wstring value;

		[[nodiscard]] std::type_index value_type() const noexcept override { return typeid(std::wstring); }

		bool write_text(const std::wstring& next) override {
			if (value == next) return true;
			value = next;
			notify_observers();
			return true;
		}
		[[nodiscard]] bool read_text(std::wstring& out) const override {
			out = value;
			return true;
		}
	};

	template<class T>
	struct RefState<T*> final : RefStateBase {
		T* value = nullptr;

		[[nodiscard]] std::type_index value_type() const noexcept override { return typeid(T*); }

		bool write_pointer(const std::type_info& type, void* pointer) override {
			if (type != typeid(T*)) return false;
			T* next = static_cast<T*>(pointer);
			if (value == next) return true;
			value = next;
			notify_observers();
			return true;
		}
		[[nodiscard]] bool read_pointer(const std::type_info& type, void*& out) const override {
			if (type != typeid(T*)) return false;
			out = value;
			return true;
		}
	};

	template<class T>
	class Ref : public w32ReactiveObject {
	public:
		using state_type = RefState<T>;

		Ref() : state_(std::make_shared<state_type>()) {}
		Ref(const T& value) : state_(std::make_shared<state_type>()) { state_->value = value; }

		Ref(const Ref&) = default;
		Ref(Ref&&) noexcept = default;
		Ref& operator=(const Ref&) = default;
		Ref& operator=(Ref&&) noexcept = default;

		Ref& operator=(const T& value) {
			set(value);
			return *this;
		}

		operator T() const { return value(); }

		[[nodiscard]] const T& value() const {
			track(*state_);
			return state_->value;
		}

		void set(const T& value) {
			if (state_->value == value) return;
			state_->value = value;
			state_->notify_observers();
		}

		[[nodiscard]] Ref toRef() const { return *this; }

		[[nodiscard]] RefHandle handle() const noexcept { return RefHandle(state_); }
		[[nodiscard]] bool shares_state_with(const Ref& other) const noexcept { return state_ == other.state_; }

	private:
		std::shared_ptr<state_type> state_;
	};
}
