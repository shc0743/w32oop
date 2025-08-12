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
#include "../Utility/RAII.hpp"
#pragma region internal macros
#define package namespace
#define declare {
#define endpackage }
#pragma endregion

#ifdef _MSVC_LANG
#if !(_MSVC_LANG > 201703L)
#error "Window.hpp needs C++20 or later."
#endif
#elif defined __cplusplus
#if !(__cplusplus > 201703L)
#error "Window.hpp needs C++20 or later."
#endif
#else
#error "No C++ supported"
#endif


package w32oop::ui declare;

std::vector<HWND> GetAllChildWindows(HWND hParent);

// @deprecated: this version number will no longer be updated.
constexpr long version = 50605020;
// @deprecated: this version string will no longer be updated.
const char* version_string();
// V5.6 Paralogism

using namespace exceptions;

class Window;

class EventData : public w32GUIObject {
public:
	EventData();
	EventData(
		HWND hwnd, ULONGLONG message,
		WPARAM wParam, LPARAM lParam,
		Window* source = nullptr
	);
	virtual ~EventData() = default;
public:
	HWND hwnd;
	ULONGLONG message;
	WPARAM wParam;
	LPARAM lParam;
	bool bubble;
private:
	bool isTrusted;
	Window* _source;
public:
	void returnValue(LRESULT _) {
		_defaultPrevented = true;
		result = _;
	}
	LRESULT returnValue() const {
		return result;
	}
	void preventDefault() {
		_defaultPrevented = true;
	}
	void stopPropagation() {
		_propagationStopped = true;
	}
private:
	LRESULT result;
	bool _defaultPrevented;
	bool _propagationStopped;
	bool isNotification;
public:
	bool defaultPrevented() const {
		return _defaultPrevented;
	}
	bool propagationStopped() const {
		return _propagationStopped;
	}
	bool is_notification() const {
		return isNotification;
	}
	Window* source() const {
		return _source;
	}
public:
	friend class Window;
};


class Window : public w32Window {
public:
	enum GlobalOptions {
		Option_Unknown = 0,
		Option_DebugMode,
		Option_DisableDialogWindowHandling,
		Option_DisableAcceleratorHandling,
		Option_HACCEL,
		Option_EnableHotkey,
		Option_EnableGlobalHotkey,
		Option_QuitWhenWindowAllClosed,
	};
	using msg_t = ULONGLONG;
protected:
	class HotKeyOptions {
	public:
		bool ctrl = false;
		bool shift = false;
		bool alt = false;
		int vk = 0;
		enum Scope {
			Windowed,
			Thread,
			Process,
			System
		};
		Scope scope = Thread;
		bool operator<(const HotKeyOptions& other) const {
			if (ctrl != other.ctrl) return ctrl < other.ctrl;
			if (shift != other.shift) return shift < other.shift;
			if (alt != other.alt) return alt < other.alt;
			if (vk != other.vk) return vk < other.vk;
			if (scope != other.scope) return scope < other.scope;
			return source < other.source;
		}
	protected:
		Window* source = nullptr;
		friend class Window;
	};
	class HotKeyProcData {
	public:
		function<void()> preventDefault;
		WPARAM wParam = 0;
		LPARAM lParam = 0;
		PKBDLLHOOKSTRUCT pKbdStruct = nullptr;
		Window* source = nullptr;
	};
private:
	static unordered_map<HWND, Window*> managed; // Internal -- DO NOT access it
	static recursive_mutex default_font_mutex;
	static HFONT default_font;
	static map<GlobalOptions, long long> global_options;
	static map<HotKeyOptions, function<void(HotKeyProcData&)>> hotkey_handlers;
	static std::recursive_mutex hotkey_handlers_mutex;

protected:
	HWND hwnd = nullptr; // 窗口句柄
	
public:
	static inline void set_global_option(GlobalOptions option, long long value) {
		global_options[option] = value;
	}
	static inline long long get_global_option(GlobalOptions option) {
		if (global_options.contains(option)) {
			return global_options[option];
		}
		return 0;
	}

public:
	virtual const wstring get_class_name() const;

protected:
	virtual const HICON get_window_icon() const {
		return NULL;
	}
	virtual const COLORREF get_window_background_color() const {
		return RGB(255, 255, 255);
	}
	// 虚函数：判断窗口类是否已注册（控件类可覆盖此方法）
	virtual bool class_registered() const {
		WNDCLASSEXW wc{};
		return (bool)GetClassInfoExW(GetModuleHandleW(NULL), class_name.c_str(), &wc);
	}
	virtual HFONT get_font() {
		if (default_font) return default_font;
		set_default_font();
		return default_font;
	}

public:
	static void set_default_font(HFONT font);
	static void set_default_font(wstring font_name = L"Consolas");
	static void set_accelerator(HACCEL accelerator);

private:
	wstring class_name;
	virtual void register_class_if_needed();
	DWORD _owner = 0;

protected:
	// 检查窗口句柄有效性
	inline void validate_hwnd() const {
		if (!hwnd) throw window_not_initialized_exception();
	}
	inline bool is_alive() const {
		return hwnd && IsWindow(hwnd);
	}

	virtual HWND new_window();

	class setup_info_class {
	public:
		wstring title;
		int width = 0, height = 0, x = 0, y = 0;
		LONG style = WS_OVERLAPPED, styleEx = 0; 
		HMENU hMenu = 0;
	} *setup_info;

	virtual void transfer_ownership() final {
		_owner = GetCurrentThreadId();
	}

private:
	bool _created = false;
	bool is_main_window = false;

public:
	Window(
		const std::wstring& title,
		int width,
		int height,
		int x = 0,
		int y = 0,
		LONG style = WS_OVERLAPPED,
		LONG styleEx = WS_EX_CONTROLPARENT,
		HMENU hMenu = nullptr
	);
	Window();
	virtual ~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	Window(Window&& other) noexcept :
		hwnd(other.hwnd)
		,setup_info(other.setup_info)
		//,notification_router(other.notification_router)
	{
		other.hwnd = nullptr;
		//other.notification_router = nullptr;
		if (hwnd) {
			SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
			managed[hwnd] = this;
		}
	}
	Window& operator=(Window&& other) noexcept;

	virtual operator HWND() const final {
		return hwnd;
	}

	virtual DWORD owner() const final {
		return _owner;
	}

public:
	long long user = 0; // 用户数据

public:
	virtual void create() final;
	virtual void create(
		const std::wstring& title,
		int width,
		int height,
		int x = 0,
		int y = 0,
		LONG style = 0,
		LONG styleEx = 0,
		HMENU hMenu = nullptr
	) final;
	virtual bool created() final;

	// 添加子窗口（类似appendChild）
	virtual void append(const Window& child) {
		validate_hwnd();
		SetParent(child.hwnd, hwnd);
	}

	virtual bool has_parent() final {
		validate_hwnd();
		return GetParent(hwnd);
	}
	virtual Window& parent() final {
		validate_hwnd();
		HWND parent = GetParent(hwnd);
		if (!parent) throw window_has_no_parent_exception();
		if (managed.contains(parent)) return *(managed.at(parent));
		throw window_has_no_parent_exception();
	}

	// 窗口操作方法
	virtual void update() {
		validate_hwnd();
		UpdateWindow(hwnd);
	}

	inline bool focus() {
		validate_hwnd();
		bool success = SetForegroundWindow(hwnd);
		SetFocus(hwnd);
		return success;
	}

	inline void blur() {
		validate_hwnd();
		SetFocus(GetDesktopWindow());
	}

	virtual bool force_focus(DWORD timeout = 10000) final;

	inline void move_to(int x, int y) {
		validate_hwnd();
		SetWindowPos(hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	inline void resize(int w, int h) {
		validate_hwnd();
		SetWindowPos(hwnd, nullptr, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	inline void resize(int x, int y, int w, int h) {
		validate_hwnd();
		SetWindowPos(hwnd, nullptr, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	inline void resize(const RECT& rc) {
		resize(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
	}

	virtual void center();
	static void center(HWND, HWND parent = NULL);

	inline void set_topmost(bool isTopmost) {
		validate_hwnd();
		SetWindowPos(hwnd, isTopmost ? HWND_TOPMOST : HWND_NOTOPMOST,
			0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	inline void enable(bool enabled = true) {
		validate_hwnd();
		EnableWindow(hwnd, enabled);
	}
	inline void disable() { enable(false); }

	virtual void close(bool async = true);

	// 窗口显示系列函数
	inline void show(int nCmdShow = SW_SHOW) {
		validate_hwnd();
		ShowWindow(hwnd, nCmdShow);
		UpdateWindow(hwnd);
	}
	inline void hide() {
		show(SW_HIDE);
	}
	inline void minimize() {
		show(SW_MINIMIZE);
	}
	inline void maximize() {
		show(SW_MAXIMIZE);
	}

	virtual void set_main_window(bool isMainWindow = true) final;

	virtual HMENU sysmenu() const;

	virtual wstring text() const;
	virtual void text(const std::wstring& text);
	virtual void text(const std::string& text);

	virtual HFONT font() const;
	virtual void font(HFONT font);

	virtual void add_style(LONG_PTR style) final;
	virtual void remove_style(LONG_PTR style) final;
	virtual void add_style_ex(LONG_PTR styleEx) final;
	virtual void remove_style_ex(LONG_PTR styleEx) final;

protected:
	virtual void destroy() {
		validate_hwnd();
		DestroyWindow(hwnd);
	}

	virtual void override_style(LONG_PTR style) final;
	virtual void override_style_ex(LONG_PTR styleEx) final;

public:
	// 发送消息到窗口
	virtual LRESULT send(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0) const;
	virtual BOOL post(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0) const;
	// 简化的事件模型，暂时只有冒泡（bubble）模式，不支持捕获（capture）模式
	virtual LRESULT dispatchEvent(EventData data) final;
protected:
	// 默认处理程序
	virtual LRESULT default_handler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	virtual LRESULT dispatchEvent(EventData& data, bool isTrusted, bool shouldBubble) final;
	virtual void dispatchEventForWindow(EventData& data) final;
public:
	// 主消息循环。**必须**使用此函数，而不是自定义的消息循环，
	// 因为此函数将处理一些内部细节
	static int run();
	// 为对话框窗口而设计的 run 函数。传入对话框实例，以实现
	// 模态框效果。
	static int run(Window* dialog);

protected:
	virtual void onCreated();
	virtual void onDestroy();

private:
	virtual void m_onCreated() final;
	// 静态消息处理函数
	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	// 消息处理函数
	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT dispatchMessageToWindowAndGetResult(msg_t msg, WPARAM wParam, LPARAM lParam, bool isNotification = false);

	LRESULT destroy_handler_internal(WPARAM wParam, LPARAM lParam);

	using EventRouter = unordered_map<msg_t,
		std::vector<
			std::function<void(EventData&)>
		>
	>;
	EventRouter router;
	recursive_mutex router_lock;

protected:
	// 注册事件处理器
	virtual void addEventListener(msg_t msg, function<void(EventData&)> handler) final;
	virtual void removeEventListener(msg_t msg) final;
	virtual void removeEventListener(msg_t msg, function<void(EventData&)> handler) final;

	virtual void setup_event_handlers() = 0;

private:
	// 快捷键相关功能
	class HotKeyProcInternal {
	public:
		HHOOK hHook = NULL;
		DWORD thread_id = 0;
	};
	static atomic<size_t> hotkey_global_count;
	static bool hotkey_handler_contains(bool ctrl, bool shift, bool alt, int vk_code, HotKeyOptions::Scope scope, Window* source = nullptr);
	static LRESULT __stdcall handlekb(
		int vk, bool ctrl, bool alt, bool shift,
		PKBDLLHOOKSTRUCT pkb,
		int code, WPARAM wParam, LPARAM lParam,
		HotKeyProcInternal* data
	);
	static LRESULT CALLBACK keyboard_proc(
		int    code,
		WPARAM wParam,
		LPARAM lParam,
		long long userdata
	);
	static LRESULT CALLBACK keyboard_proc_LL(
		int    code,
		WPARAM wParam,
		LPARAM lParam,
		long long userdata
	);
	static bool hook_is_LL;
	using MyHookProc = LRESULT(__stdcall*)(int code, WPARAM wParam, LPARAM lParam, long long userdata);
protected:
	// 注意：快捷键支持必须
	// - 要么在 Window::run() 之前调用register_hot_key
	// - 要么提前设置 Window::set_global_option(
	//		Window::Option_EnableHotkey （或者如果应用程序需要使用全局快捷键，还需要设置Option_EnableGlobalHotkey）
	//   , true)
	// 否则快捷键将不会生效，因为SetWindowsHookEx在Window::run开头被调用。
	// 出于性能考虑，我们默认不会启用快捷键（因为SetWindowsHookEx有性能开销）
	// 因此如果需要使用快捷键功能，请务必注意调用顺序。
	// 
	// 备注：register_hot_key内部会自动设置Option_EnableHotkey为true，因此若**在Window::run之前**
	// 调用register_hot_key，则不需要再调用set_global_option。
	// 如果是运行时添加快捷键，则需要提前调用set_global_option。
	virtual void register_hot_key(
		bool ctrl, bool alt, bool shift,
		int vk_code,
		function<void(HotKeyProcData&)> callback,
		HotKeyOptions::Scope scope = HotKeyOptions::Scope::Thread
	) final;
	virtual void remove_hot_key(
		bool ctrl, bool alt, bool shift,
		int vk_code
	) final {
		remove_hot_key(ctrl, alt, shift, vk_code, HotKeyOptions::Scope::Windowed);
		remove_hot_key(ctrl, alt, shift, vk_code, HotKeyOptions::Scope::Thread);
		remove_hot_key(ctrl, alt, shift, vk_code, HotKeyOptions::Scope::Process);
		remove_hot_key(ctrl, alt, shift, vk_code, HotKeyOptions::Scope::System);
	}
	virtual void remove_hot_key(
		bool ctrl, bool alt, bool shift,
		int vk_code,
		HotKeyOptions::Scope scope,
		Window* source = nullptr
	) final;
	virtual void remove_all_hot_key_on_window() final;
	virtual void remove_all_hot_key_global() final;
};

#pragma region macros to simplify the event handling
// DEPRECATED!! This macro makes code confusing and causes VCR001 Warning.
// Please directly *override* the setup_event_handlers
// virtual void setup_event_handlers() override
#define WINDOW_EVENT_HANDLER_DECLARE_BEGIN() virtual void setup_event_handlers() override {
// DEPRECATED!! This macro makes code confusing.
// Please directly }
#define WINDOW_EVENT_HANDLER_DECLARE_END() }

// Not necessary to super IF you DIRECTLY inherits the Window
// If your parent class DO SOMETHING in the setup_event_handlers, you will need to super
#define WINDOW_EVENT_HANDLER_SUPER(base_class) base_class::setup_event_handlers();

#define WINDOW_add_handler(msg,handler) addEventListener(msg, [this](EventData& data) { if (data.hwnd != this->hwnd) return;handler(data); });
#define WINDOW_add_notification_handler(msg,handler) addEventListener((::w32oop::ui::WINDOW_NOTIFICATION_CODES) + (msg), [this](EventData& data) { if (data.hwnd != this->hwnd || (!data.is_notification())) return;handler(data); });
#pragma endregion


#pragma region My Foundation Classes
#define w32oop_ui_foundation_add_mover(c, base) c& operator=(c&& other) noexcept {base::operator=(std::move(other));return *this;};

class BaseSystemWindow : public Window {
public:
	BaseSystemWindow(HWND parent, const std::wstring& title, int width, int height, int x = 0, int y = 0, LONG style = WS_OVERLAPPED, LONG styleEx = 0, unsigned long long ctlid_p = 0)
		: ctlid(ctlid_p != 0 ? ctlid_p : ++ctlid_generator), Window(title, width, height, x, y, style, styleEx, HMENU(ctlid_p != 0 ? ctlid_p : static_cast<decltype(ctlid_p)>(ctlid_generator)))
	{
		this->parent_window = parent;
	}
	BaseSystemWindow& operator=(BaseSystemWindow&& other) noexcept {
		Window::operator=(std::move(other));
		this->parent_window = other.parent_window;
		this->ctlid = other.ctlid;
		return *this;
	};;
	virtual void set_parent(HWND parent) {
		this->parent_window = parent;
	}
	virtual void set_parent(Window* pParent) {
		if (pParent) this->parent_window = *pParent;
		else this->parent_window = nullptr;
	}
protected:
	static std::atomic<unsigned long long> ctlid_generator;
	unsigned long long ctlid;
	HWND parent_window;
	bool class_registered() const override {
		return true;
	}
	HWND new_window() override;
	// 注意，对已经注册的Win32控件类，无法使用RegisterClassExW
	// 也就是说，我们的WndProc将不会被调用
	// 因此只能使用WINDOW_add_notification_handler而不是WINDOW_add_handler
	virtual void setup_event_handlers() override {
		// 此为顶层控件类，不需要继续super
	}

public:
	using CEventHandler = function<void(EventData&)>;
	virtual BaseSystemWindow& on(msg_t event, CEventHandler handler) {
		addEventListener((::w32oop::ui::WINDOW_NOTIFICATION_CODES)+(event),
			[this, handler](EventData& data) {
				if (data.hwnd != this->hwnd || (!data.is_notification())) return; handler(data);
			});;
		return *this;
	}
	virtual BaseSystemWindow& un(msg_t event) {
		removeEventListener((::w32oop::ui::WINDOW_NOTIFICATION_CODES)+(event));
		return *this;
	}
};

package foundation declare;

class Static : public BaseSystemWindow {
public:
	static const LONG STYLE = WS_CHILD | WS_VISIBLE;
	Static(HWND parent, const std::wstring& text, int width, int height, int x = 0, int y = 0, LONG style = STYLE)
		: BaseSystemWindow(parent, text, width, height, x, y, style) {
	}
	Static() : BaseSystemWindow(0, L"", 0, 0, 1, 1, STYLE) {}
	w32oop_ui_foundation_add_mover(Static, BaseSystemWindow);
	~Static() override {}
protected:
	const wstring get_class_name() const override {
		return L"Static";
	}
protected:
	virtual void setup_event_handlers() override {
		WINDOW_EVENT_HANDLER_SUPER(BaseSystemWindow);
	}
};

class Edit : public BaseSystemWindow {
public:
	static const LONG STYLE = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL;
	Edit(HWND parent, const std::wstring& text, int width, int height, int x = 0, int y = 0, LONG style = STYLE)
		: BaseSystemWindow(parent, text, width, height, x, y, style) {
	}
	Edit() : BaseSystemWindow(0, L"", 0, 0, 1, 1, STYLE) {}
	w32oop_ui_foundation_add_mover(Edit, BaseSystemWindow);
	~Edit() override {}
	void onChange(CEventHandler handler) {
		onChangeHandler = handler;
	}
	void undo() {
		validate_hwnd();
		Edit_Undo(hwnd);
	}
	void redo() {
		undo(); // win32控件的迷惑设计。。。参考：https://learn.microsoft.com/zh-cn/windows/win32/controls/em-undo
	}
	void max_length(int length) {
		validate_hwnd();
		if (length < 0) throw std::invalid_argument("Length must be greater than 0");
		Edit_LimitText(hwnd, length);
	}
	TCHAR password_char() {
		validate_hwnd();
		return Edit_GetPasswordChar(hwnd);
	}
	void password_char(TCHAR ch) {
		validate_hwnd();
		Edit_SetPasswordChar(hwnd, ch);
	}
	size_t line_count() {
		validate_hwnd();
		return Edit_GetLineCount(hwnd);
	}
	wstring get_line(int line) {
		validate_hwnd();
		wchar_t* buffer = new wchar_t[16384];
		Edit_GetLine(hwnd, line, buffer, 16384);
		wstring text(buffer);
		delete[] buffer;
		return text;
	}
	bool readonly() {
		validate_hwnd();
		return is_readonly;
	}
	void readonly(bool readonly) {
		validate_hwnd();
		Edit_SetReadOnly(hwnd, readonly);
		is_readonly = readonly;
	}
protected:
	const wstring get_class_name() const override {
		return L"Edit";
	}
private:
	bool is_readonly = false;
private:
	CEventHandler onChangeHandler;
	void onEditChanged(EventData& data) {
		if (onChangeHandler) {
			data.preventDefault();
			onChangeHandler(data);
		}
	}
protected:
	virtual void setup_event_handlers() override {
		WINDOW_EVENT_HANDLER_SUPER(BaseSystemWindow);
		WINDOW_add_notification_handler(EN_CHANGE, onEditChanged);
	}
};

class Button : public BaseSystemWindow {
public:
	static const LONG STYLE = WS_CHILD | BS_CENTER | BS_PUSHBUTTON | WS_VISIBLE | WS_TABSTOP;
	Button(HWND parent, const std::wstring& text, int width, int height, int x = 0, int y = 0, int ctlid = 0, LONG style = STYLE)
		: BaseSystemWindow(parent, text, width, height, x, y, style, ctlid) {}
	Button() : BaseSystemWindow(0, L"", 0, 0, 1, 1, STYLE) {}
	w32oop_ui_foundation_add_mover(Button, BaseSystemWindow);
	~Button() override {}
	void onClick(CEventHandler handler) {
		onClickHandler = handler;
	}
protected:
	const wstring get_class_name() const override {
		return L"Button";
	}
private:
	CEventHandler onClickHandler;
	void onBtnClicked(EventData& data) {
		if (onClickHandler) {
			data.preventDefault();
			onClickHandler(data);
		}
	}
protected:
	// for Win32 controls, we use the notification instead of the event
	virtual void setup_event_handlers() override {
		WINDOW_EVENT_HANDLER_SUPER(BaseSystemWindow);
		WINDOW_add_notification_handler(BN_CLICKED, onBtnClicked);
	}
};

class CheckBox : public Button {
public:
	static constexpr LONG STYLE = WS_CHILD | BS_AUTOCHECKBOX | WS_VISIBLE | WS_TABSTOP;
	CheckBox(HWND parent, const std::wstring& text, int width, int height, int x = 0, int y = 0, int ctlid = 0, LONG style = STYLE)
		: Button(parent, text, width, height, x, y, ctlid, style) {
	}
	CheckBox() : Button(0, L"", 0, 0, 1, 1, 0, STYLE) {}
	w32oop_ui_foundation_add_mover(CheckBox, Button);
	void onCreated() {
		Button::onCreated();
	}
	inline bool checked() {
		validate_hwnd();
		return Button_GetCheck(hwnd) == BST_CHECKED;
	}
	inline void check(bool checked = true) {
		validate_hwnd();
		Button_SetCheck(hwnd, checked ? BST_CHECKED : BST_UNCHECKED);
	};
	inline void uncheck() {
		check(false);
	}
	void onChanged(CEventHandler handler) {
		onChangeHandler = handler;
	}
protected:
	virtual void setup_event_handlers() override {
		WINDOW_EVENT_HANDLER_SUPER(Button);
		WINDOW_add_notification_handler(BN_CLICKED, onBtnChecked);
	}
private:
	CEventHandler onChangeHandler;
	void onBtnChecked(EventData& data) {
		if (onChangeHandler) {
			data.preventDefault();
			onChangeHandler(data);
		}
	}
};

class StatusBar : public BaseSystemWindow {
public:
	static const LONG STYLE = WS_CHILD | WS_VISIBLE;
	StatusBar(HWND parent, const std::wstring& text, int width, int height, int x = 0, int y = 0, LONG style = STYLE)
		: BaseSystemWindow(parent, text, width, height, x, y, style) {
	}
	StatusBar() : BaseSystemWindow(0, L"", 0, 0, 1, 1, STYLE) {}
	w32oop_ui_foundation_add_mover(StatusBar, BaseSystemWindow);
	~StatusBar() override {}
	inline void simple(bool isSimple) {
		send(SB_SIMPLE, isSimple ? TRUE : FALSE, 0);
	}
	wstring text() const override {
		return get_text(0);
	}
	wstring get_text(int part = 0) const {
		int len = LOWORD(send(SB_GETTEXTLENGTH, part, 0));
		if (!len || len < 0 || len > 32768) return wstring();
		auto buffer = std::make_unique<WCHAR[]>(static_cast<size_t>(len) + 1);
		send(SB_GETTEXTW, part, (LPARAM)buffer.get());
		return buffer.get();
	}
	// Note: To operate a non-simple status bar, 
	// please use `set_text(int index, const wstring& t)` instead.
	void text(const wstring& t) override {
		return set_text(SB_SIMPLEID, t);
	}
	void set_text(int index, const wstring& t) {
		send(SB_SETTEXT, LOBYTE(index), (LPARAM)t.c_str());
	}
	void set_parts(int count, int* widths) {
		send(SB_SETPARTS, count, (LPARAM)widths);
	}
protected:
	const wstring get_class_name() const override {
		return STATUSCLASSNAMEW;
	}
protected:
	virtual void setup_event_handlers() override {
		WINDOW_EVENT_HANDLER_SUPER(BaseSystemWindow);
	}
};

#undef w32oop_ui_foundation_add_mover
endpackage;
#pragma endregion



endpackage;

#undef package
#undef declare
#undef endpackage


