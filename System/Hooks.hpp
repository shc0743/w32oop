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
#include <memory>


namespace w32oop::exceptions {
	w32oop_declare_exception_class_from(invalid_hook_handle, system_exception);
}


namespace w32oop::def {
	using w32HookHandle = w32BaseHandle<HHOOK, false, UnhookWindowsHookEx, exceptions::invalid_hook_handle_exception>;
	
	class w32HookObject : public w32SystemObject {
	public:
		w32HookObject() = default;
		~w32HookObject() = default;
	};
}


namespace w32oop::system {
	namespace detail {
		struct VirtualFreeDeleter {
			void operator()(void* p) const noexcept { VirtualFree(p, 0, MEM_RELEASE); }
		};
	}
	
	class Hook : public w32HookObject {
	public:
		Hook() {
			void* memory = create_proc(cb, reinterpret_cast<long long>(this));
			callbackMemory.reset(memory);
			ptrCallback = reinterpret_cast<HOOKPROC>(memory);
		}
		
		virtual ~Hook() = default;

		Hook(const Hook&) = delete;
		Hook& operator=(const Hook&) = delete;
		
		Hook(Hook&& other) noexcept
			: callbackMemory(std::move(other.callbackMemory))
			, hHook(std::move(other.hHook))
			, ptrCallback(other.ptrCallback)
		{
			other.ptrCallback = nullptr;
		}
		
		Hook& operator=(Hook&& other) noexcept {
			if (this != &other) {
				callbackMemory = std::move(other.callbackMemory);
				hHook = std::move(other.hHook);
				ptrCallback = other.ptrCallback;
				other.ptrCallback = nullptr;
			}
			return *this;
		}

		virtual void set(int idHook, DWORD dwThreadId, HINSTANCE hMod = NULL) final;
		virtual void unset() final;

	protected:
		std::unique_ptr<void, detail::VirtualFreeDeleter> callbackMemory;
		w32HookHandle hHook;
		HOOKPROC ptrCallback;
		
	protected:
		using MyHookProc = LRESULT(__stdcall*)(int code, WPARAM wParam, LPARAM lParam, long long userdata);
		static LRESULT CALLBACK cb(int nCode, WPARAM wParam, LPARAM lParam, long long userdata);
		static void* create_proc(MyHookProc pfn, long long userdata);

	protected:
		virtual LRESULT callback(int nCode, WPARAM wParam, LPARAM lParam) = 0;
	};
}
