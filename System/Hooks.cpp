#include "Hooks.hpp"
#include "../Utility/Memory.hpp"


LRESULT w32oop::system::Hook::cb(int nCode, WPARAM wParam, LPARAM lParam, long long userdata) {
	if (!userdata) return 1;

	Hook* target = (Hook*)userdata;
	if (!(util::memory::MemoryChecker<sizeof(Hook)>().readable(target))) return 1;

	return target->callback(nCode, wParam, lParam);
}

BOOL __stdcall w32oop::system::MyUnhookWindowsHookEx(HHOOK hhk) {
	static HMODULE k32 = GetModuleHandleW(L"kernel32.dll");
	if (!k32) __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE);
	static HMODULE u32 = GetModuleHandleW(L"user32.dll");
	if (!u32) __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE);
	static auto GetProcAddress = reinterpret_cast<decltype(&::GetProcAddress)>(::GetProcAddress(k32, "GetProcAddress"));
	if (!GetProcAddress) __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE);
	static auto UnhookWindowsHookEx = reinterpret_cast<decltype(&::UnhookWindowsHookEx)>(GetProcAddress(u32, "UnhookWindowsHookEx"));
	if (!UnhookWindowsHookEx) __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE);
	return UnhookWindowsHookEx(hhk);
}

HOOKPROC w32oop::system::Hook::create_proc(MyHookProc pfn, long long userdata) {
	void* memory = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_READWRITE); // 4096是最小的了
	if (!memory) throw std::bad_alloc();
	const auto fail = [&](const char* reason) {
		VirtualFree(memory, 0, MEM_RELEASE);
		throw std::runtime_error(reason);
	};

	// 向 memory 写入我们的x86_64代码
#ifdef _WIN64
#if 0
	; x64机器码 - __stdcall函数转发器
		; 函数签名: LRESULT __stdcall function_name(int nCode, WPARAM wParam, LPARAM lParam)
		; 转发到: LRESULT __stdcall procname(int nCode, WPARAM wParam, LPARAM lParam, long long userdata)
#endif
	static const unsigned char payload[] = {
		0x53,                               // push rbx
		0x56,                               // push rsi
		0x57,                               // push rdi
		0x55,                               // push rbp
		0x48, 0x83, 0xEC, 0x28,             // sub rsp, 28h (对齐栈)
		// 保存参数（注意：lParam 已经在 R8 中！）
		0x48, 0x89, 0xCB,                   // mov rbx, rcx   ; 保存 nCode
		0x48, 0x89, 0xD6,                   // mov rsi, rdx   ; 保存 wParam
		//; lParam 已在 R8
		// 加载函数指针和用户数据
		0x48, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // mov rax, [func_ptr]
		0x49, 0xB9, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // mov r9, [userdata]
		// 设置调用参数
		0x48, 0x89, 0xD9,                   // mov rcx, rbx   ; 恢复 nCode
		0x48, 0x89, 0xF2,                   // mov rdx, rsi   ; 恢复 wParam
		0xFF, 0xD0,                         // call rax
		// 恢复寄存器
		0x48, 0x83, 0xC4, 0x28,             // add rsp, 28h
		0x5D,                               // pop rbp
		0x5F,                               // pop rdi
		0x5E,                               // pop rsi
		0x5B,                               // pop rbx
		0xC3                                // ret
	};

	SIZE_T written = 0;
	if (0 == WriteProcessMemory(GetCurrentProcess(), memory, &payload, sizeof(payload), &written) || written != sizeof(payload))
		fail("Failed to write memory");

	// 写入placeholder
	// 函数指针（占位符 8 字节）
	if (!WriteProcessMemory(GetCurrentProcess(), (char*)memory + 16, &pfn, 8, &written) || written != 8)
		fail("Failed to write function pointer");

	// userdata（占位符 8 字节）
	if (!WriteProcessMemory(GetCurrentProcess(), (char*)memory + 26, &userdata, 8, &written) || written != 8)
		fail("Failed to write userdata");
#else
	// x86 (32位) 机器码 —— __stdcall 3参数钩子跳板 → 转发到 4参数 __stdcall cb()
	// 进入时栈: [esp+4]=nCode [esp+8]=wParam [esp+0Ch]=lParam (返回地址在 [esp])
	static const unsigned char payload[] = {
			0x55,                               // push ebp
			0x89, 0xE5,                         // mov ebp, esp
			0xB8, 0xCC, 0xCC, 0xCC, 0xCC,       // mov eax, userdata
			0x6A, 0x00,                         // push 0                 ; userdata 高32位
			0x50,                               // push eax               ; userdata 低32位
			0xFF, 0x75, 0x10,                   // push dword ptr [ebp+10h] ; lParam
			0xFF, 0x75, 0x0C,                   // push dword ptr [ebp+0Ch] ; wParam
			0xFF, 0x75, 0x08,                   // push dword ptr [ebp+8]   ; nCode
			0xB8, 0xCC, 0xCC, 0xCC, 0xCC,       // mov eax, func_ptr      
			0xFF, 0xD0,                         // call eax               ; cb 自己清 20 字节栈
			0x5D,                               // pop ebp
			0xC2, 0x0C, 0x00                    // ret 0Ch                ; 清掉钩子的3个参数(12字节)
	};

	SIZE_T written = 0;
	if (0 == WriteProcessMemory(GetCurrentProcess(), memory, &payload, sizeof(payload), &written) || written != sizeof(payload))
		fail("Failed to write memory");

	// userdata 占位：偏移 4，4 字节（写入低 32 位即可，cb 里只做 (Hook*)userdata 截断）
	if (!WriteProcessMemory(GetCurrentProcess(), (char*)memory + 4, &userdata, 4, &written) || written != 4)
		fail("Failed to write userdata");

	// 函数指针占位：偏移 21，4 字节
	if (!WriteProcessMemory(GetCurrentProcess(), (char*)memory + 21, &pfn, 4, &written) || written != 4)
		fail("Failed to write function pointer");
#endif

	DWORD old_page_protection = 0;
	if (!VirtualProtect(memory, sizeof(payload), PAGE_EXECUTE_READ, &old_page_protection)) // 防止写入
		fail("Failed to change memory protection");

	return reinterpret_cast<HOOKPROC>(memory);
}

void w32oop::system::Hook::set(int idHook, DWORD dwThreadId, HINSTANCE hMod) {
	static HMODULE k32 = GetModuleHandleW(L"kernel32.dll");
	if (!k32) __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE);
	static HMODULE u32 = GetModuleHandleW(L"user32.dll");
	if (!u32) __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE);
	static auto GetProcAddress = reinterpret_cast<decltype(&::GetProcAddress)>(::GetProcAddress(k32, "GetProcAddress"));
	if (!GetProcAddress) __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE);
	static auto SetWindowsHookExW = reinterpret_cast<decltype(&::SetWindowsHookExW)>(GetProcAddress(u32, "SetWindowsHookExW"));
	if (!SetWindowsHookExW) __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE);
	hHook = SetWindowsHookExW(idHook, ptrCallback, hMod, dwThreadId);
}

void w32oop::system::Hook::unset() {
	hHook.close();
}


