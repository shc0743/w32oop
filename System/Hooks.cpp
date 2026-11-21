#include "Hooks.hpp"
#include <array>
#include <utility>
#include "../Utility/Memory.hpp"
using namespace std;


LRESULT w32oop::system::Hook::cb(int nCode, WPARAM wParam, LPARAM lParam, long long userdata) {
	if (!userdata) return 1;

	Hook* target = (Hook*)userdata;
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
		std::atomic<bool> did_;
		std::mutex mutex;
		std::string str;
		signed char password;
	public:
		es(std::string str2, signed char pass) {
			did_ = false;
			this->str = str2;
			this->password = pass;
		}
	private:
		void d() {
			if (did_) return;
			std::lock_guard gg(mutex);
			if (did_) return;
			for (auto& i : str) i = (decltype(str)::value_type)((unsigned char)(i) - (unsigned char)(password));
			did_ = true;
		}
	public:
		inline bool did() const {
			return did_;
		}
		inline const std::string& data() const {
			return str;
		}
		operator const char*() {
			if (!did_) d();
			return str.c_str();
		}
	};

public:
	HookStringTable_t() {}

public:
	es sh = es("Ykz]otju}yNuuqK~]", 6);
	es cn = es("JhssUl\x7f{OvvrL\x7f", 7);
	es uh = es("Mf`ggcOaf\\gok@ggc=p", -8);
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

#pragma region support

#define MAX_SLOTS 256

using MyHookProcType = LRESULT(CALLBACK*)(int code, WPARAM wParam, LPARAM lParam, long long userdata);
struct ProcData {
	bool slotCannotUse;
	long long userdata;
	MyHookProcType lpfn;
};
static ProcData ProcDataMap[MAX_SLOTS]{};
static SRWLOCK ProcDataMapx = SRWLOCK_INIT;

template<size_t N>
inline LRESULT CALLBACK HookThunk(int code, WPARAM wParam, LPARAM lParam) {
	AcquireSRWLockShared(&ProcDataMapx);
	long long u = ProcDataMap[N].userdata;
	bool f = ProcDataMap[N].slotCannotUse;
	MyHookProcType p = ProcDataMap[N].lpfn;
	ReleaseSRWLockShared(&ProcDataMapx);
	if (!f || !p) return CallNextHookEx(nullptr, code, wParam, lParam);
	return p(code, wParam, lParam, u);
}

template<size_t... Is>
inline constexpr std::array<HOOKPROC, sizeof...(Is)>
MakeProcTable(std::index_sequence<Is...>) {
	return { { &HookThunk<Is>... } };
}

class w32oop::system::Hook::ProcAddrMap_t {
public:
	ProcAddrMap_t() {}

	uint32_t RequestThunk(MyHookProcType lpfn, long long userdata) {
		uint32_t i = 0;
		AcquireSRWLockExclusive(&ProcDataMapx);
		for (; i < MAX_SLOTS; ++i) {
			if (ProcDataMap[i].slotCannotUse) continue;
			ProcDataMap[i].slotCannotUse = true;
			ProcDataMap[i].lpfn = lpfn;
			ProcDataMap[i].userdata = userdata;
			i = i + 1;
			goto end;
		}
		i = 0;
		end:
		ReleaseSRWLockExclusive(&ProcDataMapx);
		return i;
	}
	void ReleaseThunk(uint32_t id) {
		AcquireSRWLockExclusive(&ProcDataMapx);
		id -= 1;
		ProcDataMap[id].slotCannotUse = false;
		ProcDataMap[id].lpfn = nullptr;
		ProcDataMap[id].userdata = 0;
		ReleaseSRWLockExclusive(&ProcDataMapx);
	}
	HOOKPROC QueryThunkAddressById(uint32_t id) {
		if (id == 0 || id > MAX_SLOTS) return nullptr;
		return ProcTable[size_t(id) - 1];
	}
private:
	static constexpr auto ProcTable = MakeProcTable(std::make_index_sequence<MAX_SLOTS>{});
};
w32oop::system::Hook::ProcAddrMap_t w32oop::system::Hook::ProcAddrMap;

#pragma endregion


w32oop::system::Hook::HookProcInfo w32oop::system::Hook::create_proc(MyHookProc pfn, long long userdata) {
#ifdef W32OOP_HOOKS_ENABLE_DYNAMIC
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

	return Hook::HookProcInfo{ .ptrCallback = reinterpret_cast<HOOKPROC>(memory), .ptrCallbackId = 0 };
#else
	uint32_t id = ProcAddrMap.RequestThunk(pfn, userdata);
	if (!id) throw std::runtime_error("slot is full");
	return Hook::HookProcInfo{ .ptrCallback = ProcAddrMap.QueryThunkAddressById(id), .ptrCallbackId = id };
#endif
}

void w32oop::system::Hook::uncreate_proc(_Inout_ HookProcInfo* lpProcedure) {
	if (lpProcedure->ptrCallbackId) {
		ProcAddrMap.ReleaseThunk(lpProcedure->ptrCallbackId);
	}
	else if (lpProcedure->ptrCallback) {
		VirtualFree(lpProcedure->ptrCallback, 0, MEM_RELEASE);
	}
	lpProcedure->ptrCallback = nullptr;
	lpProcedure->ptrCallbackId = 0;
}

w32oop::system::Hook::~Hook() {
	hHook.close();
	uncreate_proc(&my_procedure);
}

void w32oop::system::Hook::set(int idHook, DWORD dwThreadId, HINSTANCE hMod) {
	if (!my_procedure.ptrCallback) my_procedure = create_proc(cb, (long long)(void*)this);

	static HMODULE u32 = GetU32();
	static auto SetWindowsHookExW = reinterpret_cast<decltype(&::SetWindowsHookExW)>(GetProcAddress(u32, str.sh));
	if (!SetWindowsHookExW) __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE);
	hHook = SetWindowsHookExW(idHook, my_procedure.ptrCallback, hMod, dwThreadId);
}

void w32oop::system::Hook::unset() {
	hHook.close();
	uncreate_proc(&my_procedure);
}


