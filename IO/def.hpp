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
	w32oop_declare_exception_class(io);
	w32oop_declare_exception_class_from(invalid_file_handle, io_exception);
}

namespace w32oop::def {
	class w32IoObject : public w32Object {
    public:
		w32IoObject() = default;
		~w32IoObject() = default;
	};
	using w32FileHandle = w32BaseHandle<HANDLE, false, CloseHandle, exceptions::invalid_file_handle_exception>;

	class w32FileObject : public w32IoObject {
    public:
		w32FileObject() = default;
		~w32FileObject() = default;
	};
	class w32PipeObject : public w32IoObject {
    public:
		w32PipeObject() = default;
		~w32PipeObject() = default;
	};

}
