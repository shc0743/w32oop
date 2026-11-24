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

#include <any>
#include <cstddef>
#include <string>
#include <string_view>
#include <typeinfo>
#include <unordered_map>
#include <utility>

namespace w32oop::util {

	class Property : public w32Object {
	public:
		Property() = default;
		~Property() = default;

		Property(const Property&) = default;
		Property& operator=(const Property&) = default;
		Property(Property&&) noexcept = default;
		Property& operator=(Property&&) noexcept = default;

		[[nodiscard]] bool has(std::string_view name) const {
			return values_.contains(std::string(name));
		}

		template<class T>
		[[nodiscard]] bool is(std::string_view name) const {
			auto it = values_.find(std::string(name));
			return it != values_.end() && it->second.type() == typeid(T);
		}

		[[nodiscard]] const std::type_info& type(std::string_view name) const {
			auto it = values_.find(std::string(name));
			return it == values_.end() ? typeid(void) : it->second.type();
		}

		template<class T>
		[[nodiscard]] T* find(std::string_view name) {
			auto it = values_.find(std::string(name));
			if (it == values_.end()) return nullptr;
			return std::any_cast<T>(&it->second);
		}

		template<class T>
		[[nodiscard]] const T* find(std::string_view name) const {
			auto it = values_.find(std::string(name));
			if (it == values_.end()) return nullptr;
			return std::any_cast<T>(&it->second);
		}

		template<class T>
		void set(std::string_view name, T&& value) {
			values_.insert_or_assign(std::string(name), std::any(std::forward<T>(value)));
		}

		bool erase(std::string_view name) {
			return values_.erase(std::string(name)) != 0;
		}

		void clear() noexcept { values_.clear(); }

		[[nodiscard]] bool empty() const noexcept { return values_.empty(); }
		[[nodiscard]] std::size_t size() const noexcept { return values_.size(); }

	private:
		std::unordered_map<std::string, std::any> values_;
	};
}
