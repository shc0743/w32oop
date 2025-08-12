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
#include "./Window.hpp"


namespace w32oop::ui {
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
	bool class_registered() const override;
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


#pragma region w32oop Foundation Classes
namespace foundation {


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
}} // namespace w32oop::ui::foundation

#pragma endregion
