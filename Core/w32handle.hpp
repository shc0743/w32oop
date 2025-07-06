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
#include "w32def.hpp"

namespace w32oop::core {
	/// <summary>
	///	The base RAII class, for all handle types
	/// </summary>
	/// <typeparam name="HandleType">The type of the handle, "HANDLE" for example.</typeparam>
	/// <typeparam name="invalid_exception_class">What will be thown if the handle is not valid?</typeparam>
	/// <typeparam name="AllowNegativeOneAsValidHandle">If the specific type accepts (-1). For example, in file handle -1 is considered as invalid, but in process handle, -1 is a pseudo handle referring to the current process itself.</typeparam>
	/// <typeparam name="HandleCloser">The function pointer to close the handle, "CloseHandle" for example.</typeparam>
	template <typename HandleType, bool AllowNegativeOneAsValidHandle, auto HandleCloser, typename invalid_exception_class>
	class w32BaseHandle : public w32RAIIObject {
		HandleType value;
	public:
		inline bool is_valid() const {
			if (!value) return false;
			if (value != reinterpret_cast<HandleType>(-1)) return true;
			if (AllowNegativeOneAsValidHandle) return true;
			return false;
		}
		inline bool is_closable() const {
			return (value != 0) && (value != reinterpret_cast<HandleType>(-1));
		}
		inline void validate() const {
			if (!is_valid()) throw invalid_exception_class("Invalid handle value");
		}

		// Constructors (Normal)
		w32BaseHandle() : value(0) {}
		// Constructors (Resource Acquisition Is Initialization)
		w32BaseHandle(HandleType value) {
			this->value = value;
			validate(); // RAII: Throws if invalid
		}
		virtual ~w32BaseHandle() {
			if (is_closable()) HandleCloser(value);
		}
		// Construct from a handle
		w32BaseHandle& operator=(HandleType value) {
			if (is_closable()) HandleCloser(this->value); // in operator=, we should close the old handle first
			this->value = value;
			// do not validate
			return *this;
		}
		// Delete the copy constructor so that we can't accidentally copy handles
		w32BaseHandle(const w32BaseHandle&) = delete;
		w32BaseHandle& operator=(const w32BaseHandle&) = delete;
		// Add move semantics so that we can move handles
		w32BaseHandle(w32BaseHandle&& other) noexcept
			: value(std::exchange(other.value, HandleType{}))
		{
		}
		w32BaseHandle& operator=(w32BaseHandle&& other) noexcept {
			if (this == &other) return *this; // self-move
			if (is_closable()) HandleCloser(this->value); // in operator=, we should close the old handle first
			this->value = other.value;
			other.value = 0;
			return *this;
		}
	public:
		// Converter
		operator HandleType() const {
			validate();
			return value;
		}
		// Getter, without validation
		HandleType get() const noexcept {
			return value;
		}
	public:
		// Closer
		void close() noexcept {
			if (is_closable()) HandleCloser(value);
			value = 0;
		}
	};
};

