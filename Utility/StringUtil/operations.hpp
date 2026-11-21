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


namespace w32oop::util::str::operations {
	/// @brief 原地替换字符串中的子串。
	inline std::string& replace(
		std::string& strBase,
		const std::string& strSrc,
		const std::string& strDes
	) {
		if (strSrc.empty()) return strBase;

		std::string::size_type pos = 0;
		const std::string::size_type srcLen = strSrc.size();
		const std::string::size_type desLen = strDes.size();

		while ((pos = strBase.find(strSrc, pos)) != std::string::npos) {
			strBase.replace(pos, srcLen, strDes);
			pos += desLen;
		}

		return strBase;
	}

	/// @brief 原地替换字符串中的子串。
	inline std::wstring& replace(
		std::wstring& strBase,
		const std::wstring& strSrc,
		const std::wstring& strDes
	) {
		if (strSrc.empty()) return strBase;

		std::wstring::size_type pos = 0;
		const std::wstring::size_type srcLen = strSrc.size();
		const std::wstring::size_type desLen = strDes.size();

		while ((pos = strBase.find(strSrc, pos)) != std::wstring::npos) {
			strBase.replace(pos, srcLen, strDes);
			pos += desLen;
		}

		return strBase;
	}

	/// @brief 返回替换后的新字符串，原串不变。
	inline std::string toReplaced(
		const std::string& strBase,
		const std::string& strSrc,
		const std::string& strDes
	) {
		std::string result = strBase;
		return replace(result, strSrc, strDes);
	}

	/// @brief 返回替换后的新字符串，原串不变。
	inline std::wstring toReplaced(
		const std::wstring& strBase,
		const std::wstring& strSrc,
		const std::wstring& strDes
	) {
		std::wstring result = strBase;
		return replace(result, strSrc, strDes);
	}

	/// @brief 分割字符串，结果写入 dest。
	inline std::vector<std::string>& split(
		const std::string& src,
		const std::string& separator,
		std::vector<std::string>& dest
	) {
		dest.clear();

		if (separator.empty()) {
			dest.push_back(src);
			return dest;
		}

		std::string::size_type start = 0;

		for (;;) {
			const std::string::size_type index = src.find(separator, start);
			if (index == std::string::npos) {
				dest.push_back(src.substr(start));
				break;
			}

			dest.push_back(src.substr(start, index - start));
			start = index + separator.size();
		}

		return dest;
	}

	/// @brief 分割字符串，结果写入 dest。
	inline std::vector<std::wstring>& split(
		const std::wstring& src,
		const std::wstring& separator,
		std::vector<std::wstring>& dest
	) {
		dest.clear();

		if (separator.empty()) {
			dest.push_back(src);
			return dest;
		}

		std::wstring::size_type start = 0;

		for (;;) {
			const std::wstring::size_type index = src.find(separator, start);
			if (index == std::wstring::npos) {
				dest.push_back(src.substr(start));
				break;
			}

			dest.push_back(src.substr(start, index - start));
			start = index + separator.size();
		}

		return dest;
	}

	/// @brief 返回分割后的新向量。
	inline std::vector<std::string> toSplit(
		const std::string& src,
		const std::string& separator
	) {
		std::vector<std::string> result;
		split(src, separator, result);
		return result;
	}

	/// @brief 返回分割后的新向量。
	inline std::vector<std::wstring> toSplit(
		const std::wstring& src,
		const std::wstring& separator
	) {
		std::vector<std::wstring> result;
		split(src, separator, result);
		return result;
	}

	/// <summary>
	/// 移除字符串前后的空白字符。
	/// </summary>
	/// <param name="str">欲移除的字符串</param>
	inline void trim(std::string& str) {
		const std::string::size_type first = str.find_first_not_of(" \t\r\n");
		if (first == std::string::npos) {
			str.clear();
			return;
		}
		str.erase(0, first);

		const std::string::size_type last = str.find_last_not_of(" \t\r\n");
		str.erase(last + 1);
	}

	/// <summary>
	/// 移除字符串前后的空白字符。
	/// </summary>
	/// <param name="str">欲移除的字符串</param>
	inline void trim(std::wstring& str) {
		const std::wstring::size_type first = str.find_first_not_of(L" \t\r\n");
		if (first == std::wstring::npos) {
			str.clear();
			return;
		}
		str.erase(0, first);

		const std::wstring::size_type last = str.find_last_not_of(L" \t\r\n");
		str.erase(last + 1);
	}

	/// <summary>
	/// 移除字符串前后的空白字符。
	/// </summary>
	/// <param name="str">欲移除的字符串</param>
	inline std::string toTrimmed(const std::string& str) {
		std::string result = str;
		trim(result);
		return result;
	}

	/// <summary>
	/// 移除字符串前后的空白字符。
	/// </summary>
	/// <param name="str">欲移除的字符串</param>
	inline std::wstring toTrimmed(const std::wstring& str) {
		std::wstring result = str;
		trim(result);
		return result;
	}
}
