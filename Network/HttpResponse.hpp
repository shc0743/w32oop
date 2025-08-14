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
#include "../IO/def.hpp"



namespace w32oop::network {
	class HttpBody : public def::w32NetworkObject {
	public:
		virtual std::shared_ptr<HttpBody> clone() const = 0;
	};
	class HttpMemoryBody : public HttpBody {
	public:
		HttpMemoryBody() {};
		HttpMemoryBody(const std::vector<uint8_t>& data) : m_data(data) {};
		HttpMemoryBody(std::vector<uint8_t>&& data) : m_data(std::move(data)) {};
	protected:
		std::vector<uint8_t> m_data;
	public:
		std::vector<uint8_t>& data() { return m_data; }
		const std::vector<uint8_t>& data() const { return m_data; }
		std::vector<uint8_t>& arrayBuffer() { return m_data; }
		const std::vector<uint8_t>& arrayBuffer() const { return m_data; }
		wstring text() const;

		std::shared_ptr<HttpBody> clone() const override {
			return std::make_shared<HttpMemoryBody>(m_data); // 这里调用拷贝构造函数
		}
	};
	// Such a bad design... I will try to refactor it if I have time.
	class HttpFileBody : public HttpBody {
	public:
		HttpFileBody() {};
		HttpFileBody(w32FileHandle&& hFile) : hFile(std::move(hFile)) {};
		HttpFileBody(w32FileHandle& hFile) : hFile(std::move(hFile)) {};
	protected:
		w32FileHandle hFile;
	public:
		w32FileHandle& getFile() { return hFile; }
		std::shared_ptr<HttpBody> clone() const override;
	};
	// See also https://developer.mozilla.org/docs/Web/API/Response/Response
	struct HttpResponseInitOptions {
		DWORD status = 200; // 咱也不知道微软咋想的。。总之 32 位的数字
		wstring statusText = L"OK";
		HttpHeaders headers;
	};
	class HttpResponse : public def::w32NetworkObject {
	public:
		explicit HttpResponse(std::shared_ptr<HttpBody> body, HttpResponseInitOptions options = { 200 })
			: m_body(body), m_initdata(options) {};
		HttpResponse(const HttpResponse& other); // clone object
	protected:
		std::shared_ptr<HttpBody> m_body;
		HttpResponseInitOptions m_initdata;
	public:
		bool isMemoryBody() const { return nullptr != dynamic_cast<HttpMemoryBody*>(m_body.get()); }
		bool isFileBody() const { return nullptr != dynamic_cast<HttpFileBody*>(m_body.get()); }
		std::shared_ptr<HttpBody> body() { return m_body; }
		const std::shared_ptr<HttpBody> body() const { return m_body; }
		HttpMemoryBody& asMemoryBody() { return dynamic_cast<HttpMemoryBody&>(*m_body); }
		HttpFileBody& asFileBody() { return dynamic_cast<HttpFileBody&>(*m_body); }
		DWORD status() const { return m_initdata.status; }
		wstring statusText() const { return m_initdata.statusText; }
		HttpHeaders& headers() { return m_initdata.headers; }
		const HttpHeaders& headers() const { return m_initdata.headers; }
		bool ok() const { return m_initdata.status >= 200 && m_initdata.status < 300; }
		wstring text() const { return dynamic_cast<HttpMemoryBody&>(*m_body).text(); }
	};

};

