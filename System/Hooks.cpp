#include "Hooks.hpp"
#include "../Utility/Memory.hpp"


LRESULT w32oop::system::Hook::cb(int nCode, WPARAM wParam, LPARAM lParam, long long userdata) {
	if (!userdata) return 1;

	Hook* target = (Hook*)userdata;
	if (!(util::memory::MemoryChecker<sizeof(Hook)>().readable(target))) return 1;

	return target->callback(nCode, wParam, lParam);
}

static HMODULE GetU32() {
	static HMODULE u32 = nullptr;
	if (!u32) u32 = GetModuleHandleW(L"user32.dll");
	if (!u32) u32 = LoadLibraryW(L"user32.dll");
	if (!u32) __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE);
	return u32;
}

class HookStringTable_t {
	class es {
	private:
		bool did_;
		std::string str;
	public:
		es(std::string str2) {
			did_ = false;
			this->str = str2;
		}
		void d(signed char password) {
			for (auto& i : str) i -= password;
			did_ = true;
		}
		bool did() const {
			return did_;
		}
		operator const char*() const {
			return str.c_str();
		}
	};

	void dall() {
		sh.d(6);
		cn.d(7);
		uh.d(-8);
	}
public:
	HookStringTable_t() {
		dall();
	}

public:
	es sh = es("Ykz]otju}yNuuqK~]");
	es cn = es("JhssUl\x7f{OvvrL\x7f");
	es uh = es("Mf`ggcOaf\\gok@ggc=p");
};

static HookStringTable_t str;

BOOL WINAPI w32oop::system::MyUnhookWindowsHookEx(HHOOK hhk) {
	static HMODULE u32 = GetU32();
	static auto UnhookWindowsHookEx = reinterpret_cast<decltype(&::UnhookWindowsHookEx)>(GetProcAddress(u32, str.uh));
	if (!UnhookWindowsHookEx) __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE);
	return UnhookWindowsHookEx(hhk);
}

LRESULT WINAPI w32oop::system::MyCallNextHookEx(_In_opt_ HHOOK hhk, _In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	static HMODULE u32 = GetU32();
	static auto CallNextHookEx = reinterpret_cast<decltype(&::CallNextHookEx)>(GetProcAddress(u32, str.cn));
	if (!CallNextHookEx) __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE);
	return CallNextHookEx(hhk, nCode, wParam, lParam);
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

	RtlCopyMemory(memory, payload, sizeof(payload));

	// 函数指针（占位符 8 字节）
	static_assert(sizeof(pfn) == 8, "expect 8-byte function pointer on x64");
	RtlCopyMemory((char*)memory + 16, &pfn, sizeof(pfn));

	// userdata（占位符 8 字节）
	RtlCopyMemory((char*)memory + 26, &userdata, 8);

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

	RtlCopyMemory(memory, &payload, sizeof(payload));

	// userdata 占位：偏移 4，4 字节（只取低 32 位）
	static_assert(sizeof(userdata) >= 4, "userdata must be at least 4 bytes");
	RtlCopyMemory((char*)memory + 4, &userdata, 4);

	// 函数指针占位：偏移 21，4 字节（显式截断到 32 位）
	const uint32_t pfn32 = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pfn));
	RtlCopyMemory((char*)memory + 21, &pfn32, sizeof(pfn32));
#endif

	DWORD old_page_protection = 0;
	if (!VirtualProtect(memory, sizeof(payload), PAGE_EXECUTE_READ, &old_page_protection)) // 防止写入
		fail("Failed to change memory protection");

	return reinterpret_cast<HOOKPROC>(memory);
}

void w32oop::system::Hook::set(int idHook, DWORD dwThreadId, HINSTANCE hMod) {
	if (!ptrCallback) ptrCallback = create_proc(cb, (long long)(void*)this);

	static HMODULE u32 = GetU32();
	static auto SetWindowsHookExW = reinterpret_cast<decltype(&::SetWindowsHookExW)>(GetProcAddress(u32, str.sh));
	if (!SetWindowsHookExW) __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE);
	hHook = SetWindowsHookExW(idHook, ptrCallback, hMod, dwThreadId);
}

void w32oop::system::Hook::unset() {
	hHook.close();
}


