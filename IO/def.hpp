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
#define w32oop_declare_exception_class(name) class name##_exception : public w32oop_exception { public: name##_exception(string d) : w32oop_exception(d) {} name##_exception() : w32oop_exception(( "Exception: " # name )) {} }

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
	class w32FileHandle : public w32IoObject, public w32RAIIObject {
		HANDLE hFile;
    public:
		w32FileHandle(HANDLE hFile) {
			this->hFile = hFile;
			validate();
		};
		~w32FileHandle() {
			if (hFile) CloseHandle(hFile);
		};

		inline void validate() const {
			if (hFile == INVALID_HANDLE_VALUE || !hFile) throw exceptions::invalid_file_handle_exception();
		};

		operator HANDLE() const {
			validate();
			return hFile;
		};
        
		w32FileHandle(const w32FileHandle&) = delete;
        w32FileHandle& operator=(const w32FileHandle&) = delete;

        w32FileHandle(w32FileHandle&& other) noexcept {
			hFile = other.hFile;
			other.hFile = nullptr;
		};
        w32FileHandle& operator=(w32FileHandle&& other) noexcept {
			hFile = other.hFile;
			other.hFile = nullptr;
			return *this;
		};
	};
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
