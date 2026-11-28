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
#include "./Joinable.hpp"


namespace w32oop::exceptions {
	w32oop_declare_exception_class_from(event_object_operation_failed, concurrency_exception);
	w32oop_declare_exception_class_from(event_object_too_many_wait_objects, concurrency_exception);
}


namespace w32oop::concurrency {
	class EventObject : public Joinable {
	protected:
		w32EventHandle hEvent;
	public:
		EventObject(bool auto_reset = false) : hEvent(CreateEventW(nullptr, !auto_reset, false, nullptr)) {}
		EventObject(bool auto_reset, bool initial_state, PCWSTR name, LPSECURITY_ATTRIBUTES lpsa) :
			hEvent(CreateEventW(lpsa, !auto_reset, initial_state, name)) {}

		// Getters
		HANDLE get() const {
			return hEvent;
		}

		// move constructor
		EventObject(EventObject&& other) noexcept : hEvent(std::move(other.hEvent)) {}

		// move assignment operator
		EventObject& operator=(EventObject&& other) noexcept {
			if (this != &other) {
				hEvent = std::move(other.hEvent);
			}
			return *this;
		}

	public:
		// Operations
		void set();
		void reset();

		virtual void join();

		bool wait(DWORD timeout = INFINITE) const;

		inline bool signaled() const {
			return wait(0) == WAIT_OBJECT_0;
		}

		/// <summary>
		/// Wait for multiple objects, include myself.
		/// </summary>
		/// <typeparam name="Container">A container, such as std::vector. It must has `push_back` and `size` and `data` methods.</typeparam>
		/// <param name="events">The extra objects to wait.</param>
		/// <param name="timeout">The timeout to wait.</param>
		/// <param name="wait_all">Whether wait all or not.</param>
		/// <returns>The wait result. If wait was failed, an exception will be thrown instead of result.</returns>
		template <typename Container>
		DWORD wait_multiple(const Container &events, DWORD timeout = INFINITE, bool wait_all = true) const {
			Container handles(events);
			handles.push_back(hEvent);
			if (handles.size() > MAXIMUM_WAIT_OBJECTS) throw w32oop::exceptions::event_object_too_many_wait_objects_exception();
			DWORD result = WaitForMultipleObjects(handles.size(), handles.data(), wait_all, timeout);
			if (result == WAIT_FAILED) {
				throw w32oop::exceptions::event_object_operation_failed_exception("Failed to wait for event object.");
			}
			return (result);
		}
	};
}

