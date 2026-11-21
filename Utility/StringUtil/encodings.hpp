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


namespace w32oop::util::str::encodings {
	inline std::string utf16_utf8(PCWSTR utf16Str) {
		if (utf16Str == nullptr || *utf16Str == L'\0') return std::string();

		const std::size_t wlen = wcslen(utf16Str);
		if (wlen > static_cast<std::size_t>(INT_MAX)) return std::string();
		const int utf16Len = static_cast<int>(wlen);

		const int utf8Len = WideCharToMultiByte(CP_UTF8, 0, utf16Str,
			utf16Len, nullptr, 0,
			nullptr, nullptr);
		if (utf8Len <= 0) return std::string();

		std::string utf8Str(static_cast<std::size_t>(utf8Len), '\0');
		if (WideCharToMultiByte(CP_UTF8, 0, utf16Str, utf16Len,
			utf8Str.data(), utf8Len,
			nullptr, nullptr) != utf8Len) {
			return std::string();
		}
		return utf8Str;
	}

	inline std::wstring utf8_utf16(PCSTR utf8Str) {
		if (utf8Str == nullptr || *utf8Str == '\0') return std::wstring();

		const std::size_t blen = strlen(utf8Str);
		if (blen > static_cast<std::size_t>(INT_MAX)) return std::wstring();
		const int utf8Len = static_cast<int>(blen);

		const int utf16Len = MultiByteToWideChar(CP_UTF8, 0, utf8Str,
			utf8Len, nullptr, 0);
		if (utf16Len <= 0) return std::wstring();

		std::wstring utf16Str(static_cast<std::size_t>(utf16Len), L'\0');
		if (MultiByteToWideChar(CP_UTF8, 0, utf8Str, utf8Len,
			utf16Str.data(), utf16Len) != utf16Len) {
			return std::wstring();
		}
		return utf16Str;
	}

	inline std::string utf16_utf8(const std::wstring& utf16Str) {
		if (utf16Str.empty()) return std::string();
		if (utf16Str.size() > static_cast<std::size_t>(INT_MAX)) return std::string();
		const int utf16Len = static_cast<int>(utf16Str.size());

		const int utf8Len = WideCharToMultiByte(CP_UTF8, 0, utf16Str.c_str(),
			utf16Len, nullptr, 0,
			nullptr, nullptr);
		if (utf8Len <= 0) return std::string();

		std::string utf8Str(static_cast<std::size_t>(utf8Len), '\0');
		if (WideCharToMultiByte(CP_UTF8, 0, utf16Str.c_str(), utf16Len,
			utf8Str.data(), utf8Len,
			nullptr, nullptr) != utf8Len) {
			return std::string();
		}
		return utf8Str;
	}

	inline std::wstring utf8_utf16(const std::string& utf8Str) {
		if (utf8Str.empty()) return std::wstring();
		if (utf8Str.size() > static_cast<std::size_t>(INT_MAX)) return std::wstring();
		const int utf8Len = static_cast<int>(utf8Str.size());

		const int utf16Len = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(),
			utf8Len, nullptr, 0);
		if (utf16Len <= 0) return std::wstring();

		std::wstring utf16Str(static_cast<std::size_t>(utf16Len), L'\0');
		if (MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), utf8Len,
			utf16Str.data(), utf16Len) != utf16Len) {
			return std::wstring();
		}
		return utf16Str;
	}
}
