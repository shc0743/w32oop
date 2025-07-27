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
#include "./StringUtil/converts.hpp"

namespace w32oop::util {
	class ErrorChecker : public w32Object {
	protected:
		DWORD code;
		wstring msg;

	public:
		ErrorChecker(DWORD code = GetLastError()) : code(code) {
			format();
		}
		ErrorChecker& operator=(const ErrorChecker& other) {
			this->code = other.code;
			this->msg = other.msg;
		};
		ErrorChecker& operator=(const DWORD& other) {
			this->code = other;
			format();
		};

		DWORD get() const noexcept { return code; }
		void check() const {
			if (code != 0) {
				throw w32oop::exceptions::win32_exception(w32oop::util::str::converts::wstr_str(message()));
			}
		}
		void format() {
			PWSTR LocalAddress = NULL;
			if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_IGNORE_INSERTS |
				FORMAT_MESSAGE_FROM_SYSTEM, NULL,
				code, 0, (PWSTR)&LocalAddress, 0, NULL)
			) {
				SetLastError(code);
				return;
			}
			msg = LocalAddress;
			LocalFree((HLOCAL)LocalAddress);
			SetLastError(code);
		}
		wstring raw_message() const noexcept {
			return msg;
		}
		wstring message() const noexcept {
			return std::format(L"Error {}: {}", code, msg);
		}
	};
}

