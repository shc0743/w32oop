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


namespace w32oop::util::str::converts {
	inline wstring str_wstr(const string& str) {
		if (str.empty()) return wstring();
		if (str.size() > static_cast<size_t>(INT_MAX)) return wstring();

		const int n = static_cast<int>(str.size());
		int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), n, nullptr, 0);
		if (len <= 0) return wstring();

		wstring result(static_cast<size_t>(len), L'\0');
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), n, result.data(), len);
		return result;
	}

	inline string wstr_str(const wstring& wstr) {
		if (wstr.empty()) return string();
		if (wstr.size() > static_cast<size_t>(INT_MAX)) return string();

		const int n = static_cast<int>(wstr.size());
		int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), n,
			nullptr, 0, nullptr, nullptr);
		if (len <= 0) return string();

		string result(static_cast<size_t>(len), '\0');
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), n,
			result.data(), len, nullptr, nullptr);
		return result;
	}
}

