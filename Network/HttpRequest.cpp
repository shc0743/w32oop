#include "HttpRequest.hpp"
#include "../Utility/StringUtil/operations.hpp"
using namespace w32oop;
using namespace w32oop::network;

#pragma comment(lib, "winhttp.lib")

void w32oop::network::CrackUrl(const std::wstring& url, std::wstring& host, std::wstring& path, INTERNET_PORT& port, bool& secure) {
	URL_COMPONENTS urlComp = { sizeof(URL_COMPONENTS) };
	wchar_t hostBuf[256] = { 0 };
	wchar_t pathBuf[1024] = { 0 };

	urlComp.lpszHostName = hostBuf;
	urlComp.dwHostNameLength = ARRAYSIZE(hostBuf);
	urlComp.lpszUrlPath = pathBuf;
	urlComp.dwUrlPathLength = ARRAYSIZE(pathBuf);
	urlComp.dwSchemeLength = 1; // 必须设置长度才能获取协议信息

	if (!WinHttpCrackUrl(url.c_str(), (DWORD)url.length(), 0, &urlComp)) {
		throw exceptions::bad_url_exception("URL malformed");
	}

	host = std::wstring(hostBuf, urlComp.dwHostNameLength);
	path = std::wstring(pathBuf, urlComp.dwUrlPathLength);
	port = urlComp.nPort;
	secure = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);
}

HttpResponse w32oop::network::fetch(HttpRequest request) {
	std::wstring host, path;
	INTERNET_PORT port;
	bool secure;
	CrackUrl(request.url(), host, path, port, secure);

	// -- 初始化连接
	w32InternetHandle hSession = WinHttpOpen(request.userAgent().c_str(), WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, nullptr, nullptr, 0);
	w32InternetHandle hConnection = WinHttpConnect(hSession, host.c_str(), port, 0);
	w32InternetHandle hRequest = WinHttpOpenRequest(hConnection, request.method().c_str(), path.c_str(), nullptr,
		request.referrer().empty() ? WINHTTP_NO_REFERER : request.referrer().c_str(),
		WINHTTP_DEFAULT_ACCEPT_TYPES, secure ? WINHTTP_FLAG_SECURE : 0);

	// -- 设置 headers
	for (const auto& [name, value] : request.headers()) {
		std::wstring header = name + L": " + value;
		WinHttpAddRequestHeaders(hRequest, header.c_str(), static_cast<DWORD>(-1L), WINHTTP_ADDREQ_FLAG_ADD);
	}

	// -- 设置 body
	LPVOID requestBody = NULL;
	DWORD requestBodyLength = 0;
	if (request.body()) {
		if (dynamic_cast<HttpMemoryBody*>(request.body().get())) {
			requestBody = dynamic_cast<HttpMemoryBody*>(request.body().get())->data().data();
			requestBodyLength = static_cast<DWORD>(dynamic_cast<HttpMemoryBody*>(request.body().get())->data().size());
		}
		else if (dynamic_cast<HttpFileBody*>(request.body().get())) {
			// TODO: 创建文件的内存映射…
			throw exceptions::network_request_not_supported_exception("HttpFileBody not supported yet.");
		}
		else throw exceptions::network_request_not_supported_exception("Unknown HttpBody type.");
	}

	// -- 发送网络请求
	BOOL result = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		requestBody, requestBodyLength, requestBodyLength, 0);
	if (!result) throw exceptions::network_request_failed_exception("WinHttpSendRequest failed.");

	// -- 接收响应
	if (!WinHttpReceiveResponse(hRequest.get(), nullptr)) {
		throw exceptions::network_request_failed_exception("Failed to receive response");
	}

	// -- 获取状态码
	DWORD statusCode = 0;
	DWORD size = sizeof(statusCode);
	WinHttpQueryHeaders(
		hRequest.get(),
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX,
		&statusCode, &size, WINHTTP_NO_HEADER_INDEX);
	
	// -- 获取状态文本
	wchar_t statusText[256] = { 0 };
	size = sizeof(statusText);
	WinHttpQueryHeaders(
		hRequest.get(),
		WINHTTP_QUERY_STATUS_TEXT,
		WINHTTP_HEADER_NAME_BY_INDEX,
		statusText, &size, WINHTTP_NO_HEADER_INDEX);

	// -- 获取响应头
	HttpHeaders resp_headers;
	DWORD headerSize = 0;
	WinHttpQueryHeaders(
		hRequest.get(),
		WINHTTP_QUERY_RAW_HEADERS_CRLF,
		WINHTTP_HEADER_NAME_BY_INDEX,
		nullptr,
		&headerSize,
		WINHTTP_NO_HEADER_INDEX
	);
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		auto headerBuffer = std::make_unique<wchar_t[]>(headerSize / sizeof(wchar_t) + 1);
		// 第二次调用 WinHttpQueryHeaders 获取实际的响应头数据
		if (WinHttpQueryHeaders(
			hRequest.get(),
			WINHTTP_QUERY_RAW_HEADERS_CRLF,
			WINHTTP_HEADER_NAME_BY_INDEX,
			headerBuffer.get(),
			&headerSize,
			WINHTTP_NO_HEADER_INDEX
		)) {
			headerBuffer[headerSize / sizeof(wchar_t)] = L'\0';
			std::wstring headersStr(headerBuffer.get());
			// 使用split函数分割行
			std::vector<std::wstring> lines;
			util::str::operations::split(headersStr, L"\r\n", lines);
			// 解析每一行（跳过第一行，即状态行）
			for (size_t i = 1; i < lines.size(); ++i) {
				const std::wstring& line = lines[i];
				if (line.empty()) continue;
				size_t colonPos = line.find(L':');
				if (colonPos == std::wstring::npos) {
					// 没有冒号，跳过
					continue;
				}
				std::wstring name = line.substr(0, colonPos);
				std::wstring value = line.substr(colonPos + 1);
				util::str::operations::trim(name);
				util::str::operations::trim(value);
				resp_headers.push_back(make_pair(name, value));
			}
		}
	}
	else {
		throw exceptions::network_io_exception("Failed to read response.");
	}

	// -- 读取响应体
	uint64_t readTotal = 0; DWORD readBytes = 0;
	uint64_t maxRead = request.size_use_file_buffer();
	bool isFileBuffer = false;
	w32FileHandle fileHandle;
	constexpr size_t readBufferSize = 1048576; // 1MiB
	auto readBuffer = std::make_unique<char[]>(readBufferSize);
	vector<uint8_t> memoryBuffer;
	memoryBuffer.reserve(maxRead);
	while (
		WinHttpReadData(hRequest, readBuffer.get(), static_cast<DWORD>(readBufferSize), &readBytes)
		&& readBytes > 0
	) {
		readTotal += readBytes;
		ifb:
		if (isFileBuffer) {
			DWORD written = 0;
			if (!WriteFile(fileHandle, readBuffer.get(), readBytes, &written, nullptr)) {
				throw exceptions::network_io_exception("Unable to write file");
			}
			continue;
		}
		if (readTotal > maxRead) {
			if (request.file_buffer_file_name().empty()) throw exceptions::network_io_exception("File buffer size exceeded and no disk file specified");
			fileHandle = CreateFileW(request.file_buffer_file_name().c_str(), GENERIC_WRITE,
				0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			isFileBuffer = true;
			// 转移到文件缓冲区
			DWORD written = 0;
			if (!WriteFile(fileHandle, memoryBuffer.data(), (DWORD)memoryBuffer.size(), &written, nullptr)) {
				throw exceptions::network_io_exception("Unable to write memory buffer to file");
			}
			memoryBuffer.clear();
			goto ifb;
		}
		// 读取到内存缓冲区(memoryBuffer)
		memoryBuffer.insert(memoryBuffer.end(), readBuffer.get(), readBuffer.get() + readBytes);
	}

	// -- 构造响应并返回
	shared_ptr<HttpBody> body = nullptr;
	if (isFileBuffer) body = make_shared<HttpFileBody>(std::move(fileHandle));
	else body = make_shared<HttpMemoryBody>(memoryBuffer);

	return HttpResponse(body, HttpResponseInitOptions{
		.status = statusCode,
		.statusText = statusText,
		.headers = resp_headers,
	});
}
