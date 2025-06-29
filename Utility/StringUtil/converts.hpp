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
		wstring result;
		size_t len = MultiByteToWideChar(CP_ACP, 0, str.c_str(),
			(int)(str.size()), NULL, 0);
		if (len < 0) return result;
		wchar_t* buffer = new wchar_t[len + 1];
		if (buffer == NULL) return result;
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)(str.size()),
			buffer, (int)len);
		buffer[len] = '\0';
		result.append(buffer);
		delete[] buffer;
		return result;
	}
	inline string wstr_str(const wstring& wstr) {
		string result;
		size_t len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(),
			(int)(wstr.size()), NULL, 0, NULL, NULL);
		if (len < 0) return result;
		char* buffer = new char[len + 1];
		if (buffer == NULL) return result;
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(),
			(int)(wstr.size()), buffer, (int)len, NULL, NULL);
		buffer[len] = '\0';
		result.append(buffer);
		delete[] buffer;
		return result;
	}
}

