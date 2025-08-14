#include "HttpResponse.hpp"
#include "../Utility/RAII.hpp"
#include "../Utility/StringUtil/encodings.hpp"
using namespace w32oop;
using namespace w32oop::network;

std::shared_ptr<HttpBody> w32oop::network::HttpFileBody::clone() const {
	WCHAR tempDir[MAX_PATH]{}, tempFileName[MAX_PATH]{};
	GetTempPathW(MAX_PATH, tempDir);
	GetTempFileNameW(tempDir, L"rsp", 0, tempFileName);
	// w32FileHandle会自动判断句柄有效性
	w32FileHandle newTempFile = CreateFileW(tempFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL);
	LARGE_INTEGER current_fp{};
	SetFilePointerEx(hFile, LARGE_INTEGER{}, &current_fp, FILE_CURRENT);
	util::RAIIHelper r([this, current_fp] { SetFilePointerEx(hFile, current_fp, NULL, FILE_BEGIN); }); // 恢复文件指针
	SetFilePointerEx(hFile, LARGE_INTEGER{}, NULL, FILE_BEGIN); // 移动到文件开头，以复制文件
	constexpr DWORD buffer_size = 32 * 1024 * 1024; // 32MiB
	auto buffer = make_unique<char[]>(buffer_size);
	DWORD readed = 0, wrote = 0;
	while (ReadFile(hFile, buffer.get(), buffer_size, &readed, NULL) && readed) {
		if (!WriteFile(newTempFile, buffer.get(), readed, &wrote, NULL)) throw runtime_error("WriteFile failed.");
	}
	return std::make_shared<HttpFileBody>(newTempFile);
}

w32oop::network::HttpResponse::HttpResponse(const HttpResponse& other) {
	this->m_initdata = other.m_initdata;
	this->m_body = other.m_body->clone();
}

wstring w32oop::network::HttpMemoryBody::text() const {
	string str(reinterpret_cast<const char*>(m_data.data()), m_data.size());
	return util::str::encodings::utf8_utf16(str);
}
