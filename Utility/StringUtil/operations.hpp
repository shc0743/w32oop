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
    /// @brief 字符串替换。
    /// @param strBase 要进行替换的字符串。
    /// @param strSrc 要替换的子字符串。
    /// @param strDes 替换后的子字符串。
    /// @return 替换后的字符串引用。
    inline string& replace(
        string& strBase, 
        const string strSrc,
        const string strDes
    ) {
        string::size_type pos = 0;
        string::size_type srcLen = strSrc.size();
        string::size_type desLen = strDes.size();
        pos = strBase.find(strSrc, pos);
        while ((pos != string::npos)) {
            strBase.replace(pos, srcLen, strDes);
            pos = strBase.find(strSrc, (pos + desLen));
        }
        return strBase;
    }
    /// @brief 字符串替换。
    /// @param strBase 要进行替换的字符串。
    /// @param strSrc 要替换的子字符串。
    /// @param strDes 替换后的子字符串。
    /// @return 替换后的字符串引用。
    inline wstring& replace(
        std::wstring& strBase,
        const std::wstring strSrc,
        const std::wstring strDes
    ) {
        wstring::size_type pos = 0;
        wstring::size_type srcLen = strSrc.size();
        wstring::size_type desLen = strDes.size();
        pos = strBase.find(strSrc, pos);
        while ((pos != wstring::npos)) {
            strBase.replace(pos, srcLen, strDes);
            pos = strBase.find(strSrc, (pos + desLen));
        }
        return strBase;
    }
    /// @brief 分割字符串。
    /// @param src 要分割的字符串。
    /// @param separator 作为分隔符的字符。
    /// @param dest 存放分割后的字符串的vector向量。
    /// @return 分割后的字符串向量的引用。
    inline std::vector<std::string>& split(
        const std::string& src, 
        const std::string separator, 
        std::vector<std::string>& dest
    ) {
        string str = src;
        string substring;
        string::size_type start = 0, index;
        dest.clear();
        index = str.find_first_of(separator, start);
        do {
            if (index == string::npos) break;
            substring = str.substr(start, index - start);
            dest.push_back(substring);
            start = index + separator.size();
            index = str.find(separator, start);
            if (start == string::npos) break;
        } while (index != string::npos);
        
        substring = str.substr(start);
        dest.push_back(substring);
        return dest;
    }
    /// @brief 分割字符串。
    /// @param src 要分割的字符串。
    /// @param separator 作为分隔符的字符。
    /// @param dest 存放分割后的字符串的vector向量。
    /// @return 分割后的字符串向量的引用。
    inline std::vector<std::wstring>& split(
        const std::wstring& src, 
        const std::wstring separator, 
        std::vector<std::wstring>& dest
    ) {
        wstring str = src;
        wstring substring;
        wstring::size_type start = 0, index;
        dest.clear();
        index = str.find_first_of(separator, start);
        do {
            if (index == wstring::npos) break;
            substring = str.substr(start, index - start);
            dest.push_back(substring);
            start = index + separator.size();
            index = str.find(separator, start);
            if (start == wstring::npos) break;
        } while (index != wstring::npos);
        
        substring = str.substr(start);
        dest.push_back(substring);
        return dest;
    }
    
}