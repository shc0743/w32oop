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
#include "./HttpResponse.hpp"



namespace w32oop::network {
	struct HttpRequestInitOptions {
		wstring method = L"GET";
		HttpHeaders headers;
		shared_ptr<HttpBody> body;
		void* credentials; // unused
		void* cache; // unused
		void* redirect; // unused
		wstring referrer;
		wstring userAgent = L"WinHTTP HTTP Client Process";
	};
	class HttpRequest : public def::w32NetworkObject {
	public:
		HttpRequest(const wstring& input, HttpRequestInitOptions options = {}) :
			input(input), options(options) {}
	protected:
		wstring input;
		HttpRequestInitOptions options;
		const SIZE_T size_use_file_buffer_default = 128 * 1048576;
		SIZE_T size_use_file_buffer_ = size_use_file_buffer_default;
		wstring file_buffer_file_name_;
	public:
		wstring url() const { return input; }
		wstring method() const { return options.method; }
		HttpHeaders& headers() { return options.headers; }
		const HttpHeaders& headers() const { return options.headers; }
		shared_ptr<HttpBody> body() { return options.body; }
		const shared_ptr<HttpBody> body() const { return options.body; }
		void body(shared_ptr<HttpBody> body) { options.body = body; }
		wstring referrer() const { return options.referrer; }
		void referrer(wstring referrer) { options.referrer = referrer; }
		wstring userAgent() const { return options.userAgent; }
		void userAgent(wstring user_agent) { options.userAgent = user_agent; }
		SIZE_T size_use_file_buffer() const { return size_use_file_buffer_; }
		void size_use_file_buffer(SIZE_T size) { size_use_file_buffer_ = size; }
		wstring file_buffer_file_name() const { return file_buffer_file_name_; }
		void file_buffer_file_name(wstring file_name) { file_buffer_file_name_ = file_name; }
	};

	void CrackUrl(const std::wstring& url,
		std::wstring& host,
		std::wstring& path,
		INTERNET_PORT& port,
		bool& secure);

	HttpResponse fetch(HttpRequest request);
#if 0
	template<class... _Args>
	HttpResponse fetch(_Args&&... args) {
        return fetch(HttpRequest(std::forward<_Args>(args)...));
	}
#else
	inline HttpResponse fetch(const wstring& input, HttpRequestInitOptions options = {}) {
        return fetch(HttpRequest(input, options));
	}
#endif
};

