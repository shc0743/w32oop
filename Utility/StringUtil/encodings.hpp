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
		if (utf16Str == nullptr) return "";
		int utf16Len = static_cast<int>(wcslen(utf16Str));
		int utf8Len = WideCharToMultiByte(CP_UTF8, 0, utf16Str,
			utf16Len, nullptr, 0, nullptr, nullptr);
		std::string utf8Str(utf8Len, 0);  // 创建一个足够大的字符串来容纳UTF-8字符串
		WideCharToMultiByte(CP_UTF8, 0, utf16Str, utf16Len, &utf8Str[0],
			utf8Len, nullptr, nullptr);
		return utf8Str;
	}
	inline std::wstring utf8_utf16(PCSTR utf8Str) {
		if (utf8Str == nullptr) return L"";
		int utf8Len = static_cast<int>(strlen(utf8Str));
		int utf16Len = MultiByteToWideChar(CP_UTF8, 0, utf8Str,
			utf8Len, nullptr, 0);
		std::wstring utf16Str(utf16Len, 0);  // 创建一个足够大的wstring来容纳UTF-16字符串
		MultiByteToWideChar(CP_UTF8, 0, utf8Str, utf8Len,
			&utf16Str[0], utf16Len);
		return utf16Str;
	}
	inline std::string utf16_utf8(const std::wstring& utf16Str) {
		return utf16_utf8(utf16Str.c_str());
	}
	inline std::wstring utf8_utf16(const std::string& utf8Str) {
		return utf8_utf16(utf8Str.c_str());
	}
}
