#include<string>
#include<windows.h>

std::string ConvertUTF16ToUTF8(PCWSTR utf16Str);
inline std::string ConvertUTF16ToUTF8(const std::wstring& utf16Str) {
	return ConvertUTF16ToUTF8(utf16Str.c_str());
}
inline bool ConvertUTF16ToUTF8(
	const std::wstring& utf16Str, std::string& receiver
) {
	return (receiver = ConvertUTF16ToUTF8(utf16Str.c_str())).length();
}

std::wstring ConvertUTF8ToUTF16(PCSTR utf8Str);
inline std::wstring ConvertUTF8ToUTF16(const std::string& utf8Str) {
	return ConvertUTF8ToUTF16(utf8Str.c_str());
}
inline bool ConvertUTF8ToUTF16(
	const std::string& utf8Str, std::wstring& receiver
) {
	return (receiver = ConvertUTF8ToUTF16(utf8Str.c_str())).length();
}


std::string ConvertUTF16ToUTF8(PCWSTR utf16Str) {
	if (utf16Str == nullptr) return "";
	int utf16Len = static_cast<int>(wcslen(utf16Str));
	int utf8Len = WideCharToMultiByte(CP_UTF8, 0, utf16Str,
		utf16Len,	nullptr, 0, nullptr, nullptr);
	std::string utf8Str(utf8Len, 0);  // 创建一个足够大的字符串来容纳UTF-8字符串
	WideCharToMultiByte(CP_UTF8, 0, utf16Str, utf16Len, &utf8Str[0],
		utf8Len, nullptr, nullptr);
	return utf8Str;
}

std::wstring ConvertUTF8ToUTF16(PCSTR utf8Str) {
	if (utf8Str == nullptr) return L"";
	int utf8Len = static_cast<int>(strlen(utf8Str));
	int utf16Len = MultiByteToWideChar(CP_UTF8, 0, utf8Str,
		utf8Len, nullptr, 0);
	std::wstring utf16Str(utf16Len, 0);  // 创建一个足够大的wstring来容纳UTF-16字符串
	MultiByteToWideChar(CP_UTF8, 0, utf8Str, utf8Len,
		&utf16Str[0], utf16Len);
	return utf16Str;
}
