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

namespace w32oop::util::memory {
	w32oop_declare_exception_class(memory_check_failed);

	template<size_t size>
	class MemoryChecker : public w32Object {
	public:
		MemoryChecker() {};
		~MemoryChecker() {};

	public:
		bool validate(void* address) {
			return readable(address) && writable(address);
		}

		unique_ptr<char[]> read(void* address) {
			auto buffer = std::make_unique<char[]>(size);
			SIZE_T nRead = 0;
			if (!ReadProcessMemory(GetCurrentProcess(), address, buffer.get(), size, &nRead) || nRead != size) {
				throw memory_check_failed_exception();
			}
			return buffer;
		}
		bool readable(void* address) { 
			try {
				read(address);
				return true;
			}
			catch (const memory_check_failed_exception&) {
				return false;
			}
		}

		void write(void* address, const void* value) {
			SIZE_T nWritten = 0;
			if (!WriteProcessMemory(GetCurrentProcess(), address, value, size, &nWritten) || nWritten != size) {
				throw memory_check_failed_exception();
			}
		}
		bool writable(void* address) {
			try {
				auto original = read(address);
				write(address, original.get());
                return true;
			}
			catch (memory_check_failed_exception) {
				return false;
			}
		}
	};
}
