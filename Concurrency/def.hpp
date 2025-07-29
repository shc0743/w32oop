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

#include "../Core/w32def.hpp"
#include "../Core/w32handle.hpp"


namespace w32oop::exceptions {
	w32oop_declare_exception_class(concurrency);

	w32oop_declare_exception_class_from(invalid_event_handle, concurrency_exception);
	w32oop_declare_exception_class_from(invalid_process_handle, concurrency_exception);
	w32oop_declare_exception_class_from(invalid_thread_handle, concurrency_exception);
}


namespace w32oop::def {
	using w32EventHandle = w32BaseHandle<HANDLE, false, CloseHandle, exceptions::invalid_event_handle_exception>;
	using w32ProcessHandle = w32BaseHandle<HANDLE, true, w32oop::core::ClosePseudoHandle, exceptions::invalid_process_handle_exception>;
	using w32ThreadHandle = w32BaseHandle<HANDLE, true, w32oop::core::ClosePseudoHandle, exceptions::invalid_thread_handle_exception>;

    class w32ConcurrencyObject : public w32Object {
	public:
		w32ConcurrencyObject() = default;
		virtual ~w32ConcurrencyObject() = default;
	};
}


namespace w32oop::concurrency {
	
}
