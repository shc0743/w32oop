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


namespace w32oop::exceptions {
	w32oop_declare_exception_class_from(invalid_scm_handle, system_exception);
	w32oop_declare_exception_class_from(service_operation_failed, system_exception);
}

namespace w32oop::def {
	using w32ServiceHandle = w32BaseHandle<SC_HANDLE, false, CloseServiceHandle, exceptions::invalid_scm_handle_exception>;

	class w32ServiceObject : public w32SystemObject {
	public:
		w32ServiceObject() = default;
		~w32ServiceObject() = default;
	};
	class w32ServiceManagerObject : public w32SystemObject {
	public:
		w32ServiceManagerObject() = default;
		~w32ServiceManagerObject() = default;
	};
}


namespace w32oop::system {
	class Service : public w32ServiceObject {
		w32ServiceHandle hService;
	public:
		Service(w32ServiceHandle&& hService) : hService(move(hService)) {
		};
		Service(Service&& src) noexcept : hService(move(src.hService)) {
			src.hService = nullptr;
		}
		~Service() = default;
	public:
		operator SC_HANDLE() {
			return hService;
		};
	public:
		bool remove();
		bool start();
		bool control(DWORD dwControl);
		inline bool stop() { return control(SERVICE_CONTROL_STOP); };
		inline bool pause_service() { return control(SERVICE_CONTROL_PAUSE); };
		inline bool continue_service() { return control(SERVICE_CONTROL_CONTINUE); };
		inline bool restart() {
			bool ok = true;
			if (!stop()) ok = false;
			if (!start()) ok = false;
            return ok;
		}
		DWORD status();
	};

	class ServiceManager : public w32ServiceManagerObject {
		w32ServiceHandle scm;
	public:
		ServiceManager(DWORD dwDesiredAccess = SC_MANAGER_ALL_ACCESS)
			: scm(OpenSCManagerW(nullptr, nullptr, dwDesiredAccess))
		{};
	public:
		Service get(wstring name, DWORD access = SERVICE_ALL_ACCESS);
		Service create(wstring name, wstring binPath, DWORD startup_type,
			wstring displayName = L"", wstring description = L"",
			DWORD type = SERVICE_WIN32_OWN_PROCESS);
	};
}



