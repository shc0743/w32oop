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
#include <concepts>
#include <optional>


namespace w32oop::ui {
// can only use to quick find-replace; do NOT directly use as long as there are extra members
#define w32oop_ui_foundation_quick_add_mover(c, base) \
	c(c&& other) noexcept : w32oop_ui_window_move_ctor_super(base, other) {} \
	c& operator=(c&& other) noexcept { \
		w32oop_ui_window_move_assignment_start(other); \
		w32oop_ui_window_move_super(base, other); \
		w32oop_ui_window_move_assignment_end(); \
	}

namespace foundation {
	// abstract base control class
	// DO NOT super this class when setup_event_handlers!
	class ChildControlWindow : public Window {
	public:
		ChildControlWindow(HWND parent, const std::wstring& title, int width, int height, int x = 0, int y = 0, LONG style = WS_CHILD, LONG styleEx = 0, unsigned long long ctlid_p = 0)
			: ctlid(ctlid_p), Window(title, width, height, x, y, style, styleEx, HMENU(ctlid_p)), parent_window(parent) {
		}
		ChildControlWindow(ChildControlWindow&& other) noexcept :
			w32oop_ui_window_move_ctor_super(Window, other)
		{
			w32oop_ui_window_move_value(parent_window, other);
			w32oop_ui_window_move_value(ctlid, other);
		}
		ChildControlWindow& operator=(ChildControlWindow&& other) noexcept {
			w32oop_ui_window_move_assignment_start(other);
			w32oop_ui_window_move_super(Window, other);
			w32oop_ui_window_move_value(parent_window, other);
			w32oop_ui_window_move_value(ctlid, other);
			w32oop_ui_window_move_assignment_end();
		}
		virtual void set_parent(HWND parent) {
			this->parent_window = parent;
			ChildControlWindow__set_parent_final_impl(this->parent_window);
		}
		virtual void set_parent(Window* pParent) {
			if (pParent) this->parent_window = *pParent;
			else this->parent_window = nullptr;
			ChildControlWindow__set_parent_final_impl(this->parent_window);
		}
	protected:
		unsigned long long ctlid;
		HWND parent_window;
		HWND new_window() override;
	private:
		void ChildControlWindow__set_parent_final_impl(HWND hWnd) {
			if (created() && is_alive()) SetParent(this->hwnd, hWnd);
		}
	};
}

class BaseSystemWindow : public foundation::ChildControlWindow {
public:
	BaseSystemWindow(HWND parent, const std::wstring& title, int width, int height, int x = 0, int y = 0, LONG style = WS_CHILD, LONG styleEx = 0, unsigned long long ctlid_p = 0)
		: ChildControlWindow(parent, title, width, height, x, y, style, styleEx, (ctlid_p != 0 ? ctlid_p : ++ctlid_generator)) {
	}
	BaseSystemWindow(BaseSystemWindow&& other) noexcept :
		w32oop_ui_window_move_ctor_super(ChildControlWindow, other)
	{
	}
	BaseSystemWindow& operator=(BaseSystemWindow&& other) noexcept {
		w32oop_ui_window_move_assignment_start(other);
		w32oop_ui_window_move_super(ChildControlWindow, other);
		w32oop_ui_window_move_assignment_end();
	}
protected:
	static std::atomic<unsigned long long> ctlid_generator;
	bool class_registered() const override;
	virtual void setup_event_handlers() override {
		// 此为顶层控件类，不需要继续super
		// 保存原始窗口过程，以便把消息（尤其是 WM_DESTROY / WM_NCDESTROY）
		// 转发给控件本身；old_wndproc 由 Window 持有，销毁期间仍然可用。
		old_wndproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
		SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)Window::StaticWndProc);
	}

public:
	using CEventHandler = std::function<void(EventData&)>;
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
	virtual void addHandler(msg_t msg, CEventHandler handler) final {
		addEventListener(msg, handler);
	}
	virtual void removeHandler(msg_t msg, CEventHandler handler) final {
		removeEventListener(msg, handler);
	}
	virtual void removeHandler(msg_t msg) final {
		removeEventListener(msg);
	}
};


#pragma region w32oop Foundation Classes
namespace foundation {


#pragma region Standard controls (they are registered by user32 so we can directly use them)

// Basic Static class. Directly uses system Static class, no extra windows.
class Static : public BaseSystemWindow {
public:
	static const LONG STYLE = WS_CHILD | WS_VISIBLE;
	Static(HWND parent, const std::wstring& text, int width, int height, int x = 0, int y = 0, LONG style = STYLE)
		: BaseSystemWindow(parent, text, width, height, x, y, style) {
	}
	Static() : BaseSystemWindow(0, L"", 0, 0, 1, 1, STYLE) {}
	Static(Static&& other) noexcept : w32oop_ui_window_move_ctor_super(BaseSystemWindow, other) {}
	Static& operator=(Static&& other) noexcept {
		w32oop_ui_window_move_assignment_start(other);
		w32oop_ui_window_move_super(BaseSystemWindow, other);
		w32oop_ui_window_move_assignment_end();
	}
	~Static() override {}
protected:
	const std::wstring get_class_name() const override {
		return L"Static";
	}
protected:
	virtual void setup_event_handlers() override {
		WINDOW_EVENT_HANDLER_SUPER(BaseSystemWindow);
	}
};

// Extended static control, with supports like customize colors. A thin wrapper is applied.
class StaticEx : public ChildControlWindow {
public:
	static const LONG STYLE = WS_CHILD | WS_VISIBLE;
	StaticEx(HWND parent, const std::wstring& text, int width, int height, int x = 0, int y = 0, LONG style = STYLE)
		: ChildControlWindow(parent, text, width, height, x, y, style, WS_EX_CONTROLPARENT) {
	}
	StaticEx() : ChildControlWindow(0, L"", 0, 0, 1, 1, STYLE, WS_EX_CONTROLPARENT) {}
	StaticEx(StaticEx&& other) noexcept : w32oop_ui_window_move_ctor_super(ChildControlWindow, other),
		w32oop_ui_window_move_ctor_object(_MyStatic, other),
		w32oop_ui_window_move_ctor_object(bgBrush, other)
	{
		w32oop_ui_window_move_value(frontColor, other);
		w32oop_ui_window_move_value(bgColor, other);
	}
	StaticEx& operator=(StaticEx&& other) noexcept {
		w32oop_ui_window_move_assignment_start(other);
		w32oop_ui_window_move_super(ChildControlWindow, other);
		w32oop_ui_window_move_object(_MyStatic, other);
		w32oop_ui_window_move_object(bgBrush, other);
		w32oop_ui_window_move_value(frontColor, other);
		w32oop_ui_window_move_value(bgColor, other);
		w32oop_ui_window_move_assignment_end();
	}
	~StaticEx() override {}
protected:
	Static _MyStatic;
	w32BrushHandle bgBrush = CreateSolidBrush(RGB(0xf0, 0xf0, 0xf0));
	COLORREF frontColor = RGB(0, 0, 0);
	COLORREF bgColor = RGB(0xf0, 0xf0, 0xf0);
	void onSize(EventData& ev);
	void onSetText(EventData& ev);
	void onCtlColor(EventData& ev);
	virtual void setup_event_handlers() override;
public:
	constexpr COLORREF color() const { return frontColor; }
	constexpr COLORREF backgroundColor() const { return bgColor; }
	void color(COLORREF c) { frontColor = c; if (is_alive()) update(); }
	void backgroundColor(COLORREF c) { bgColor = c; bgBrush = CreateSolidBrush(c); if (is_alive()) update(); }
};

class Edit : public BaseSystemWindow {
public:
	static const LONG STYLE = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL;
	Edit(HWND parent, const std::wstring& text, int width, int height, int x = 0, int y = 0, LONG style = STYLE)
		: BaseSystemWindow(parent, text, width, height, x, y, style) {
	}
	Edit() : BaseSystemWindow(0, L"", 0, 0, 1, 1, STYLE) {}
	Edit(Edit&& other) noexcept : w32oop_ui_window_move_ctor_super(BaseSystemWindow, other),
		w32oop_ui_window_move_ctor_object(onChangeHandler, other)
	{
		w32oop_ui_window_move_value(is_readonly, other);
	}
	Edit& operator=(Edit&& other) noexcept {
		w32oop_ui_window_move_assignment_start(other);
		w32oop_ui_window_move_super(BaseSystemWindow, other);
		w32oop_ui_window_move_object(onChangeHandler, other);
		w32oop_ui_window_move_value(is_readonly, other);
		w32oop_ui_window_move_assignment_end();
	}
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
	std::size_t line_count() {
		validate_hwnd();
		return Edit_GetLineCount(hwnd);
	}
	std::wstring get_line(int line) {
		validate_hwnd();
		if (line < 0) return {};
		LRESULT lineStart = ::SendMessageW(hwnd, EM_LINEINDEX, (WPARAM)line, 0);
		if (lineStart < 0) return {};
		LRESULT lineLength = ::SendMessageW(hwnd, EM_LINELENGTH, (WPARAM)lineStart, 0);
		if (lineLength <= 0) return {};
		const size_t capacity = static_cast<size_t>(lineLength);
		std::wstring buffer(capacity + 1, L'\0');
		*reinterpret_cast<WORD*>(buffer.data()) = static_cast<WORD>(capacity);
		LRESULT copied = ::SendMessageW(hwnd, EM_GETLINE, (WPARAM)line, (LPARAM)buffer.data());
		if (copied <= 0) return {};
		buffer.resize(static_cast<size_t>(copied));
		return buffer;
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
	const std::wstring get_class_name() const override {
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
	Button(Button&& other) noexcept : w32oop_ui_window_move_ctor_super(BaseSystemWindow, other),
		w32oop_ui_window_move_ctor_object(onClickHandler, other)
	{
		w32oop_ui_window_move_value(cursor, other);
	}
	Button& operator=(Button&& other) noexcept {
		w32oop_ui_window_move_assignment_start(other);
		w32oop_ui_window_move_super(BaseSystemWindow, other);
		w32oop_ui_window_move_object(onClickHandler, other);
		w32oop_ui_window_move_value(cursor, other);
		w32oop_ui_window_move_assignment_end();
	}
	~Button() override {}
	void onClick(CEventHandler handler) {
		onClickHandler = handler;
	}
	HCURSOR cursor = NULL;
protected:
	const std::wstring get_class_name() const override {
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
	void onSetCursor(EventData& ev) {
		ev.returnValue(TRUE);
		SetCursor(cursor);
	}
protected:
	// for Win32 controls, we use the notification instead of the event
	virtual void setup_event_handlers() override {
		WINDOW_EVENT_HANDLER_SUPER(BaseSystemWindow);
		WINDOW_add_notification_handler(BN_CLICKED, onBtnClicked);
		cursor = LoadCursor(NULL, IDC_HAND);
		WINDOW_add_handler(WM_SETCURSOR, onSetCursor);
	}
};

class CheckBox : public Button {
public:
	static constexpr LONG STYLE = WS_CHILD | BS_AUTOCHECKBOX | WS_VISIBLE | WS_TABSTOP;
	CheckBox(HWND parent, const std::wstring& text, int width, int height, int x = 0, int y = 0, int ctlid = 0, LONG style = STYLE)
		: Button(parent, text, width, height, x, y, ctlid, style) {
	}
	CheckBox() : Button(0, L"", 0, 0, 1, 1, 0, STYLE) {}
	CheckBox(CheckBox&& other) noexcept : w32oop_ui_window_move_ctor_super(Button, other),
		w32oop_ui_window_move_ctor_object(onChangeHandler, other)
	{}
	CheckBox& operator=(CheckBox&& other) noexcept {
		w32oop_ui_window_move_assignment_start(other);
		w32oop_ui_window_move_super(Button, other);
		w32oop_ui_window_move_object(onChangeHandler, other);
		w32oop_ui_window_move_assignment_end();
	}
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

class ListBox : public BaseSystemWindow {
public:
	static const LONG STYLE = WS_CHILD | WS_VISIBLE | WS_TABSTOP | LBS_NOTIFY | WS_VSCROLL | WS_BORDER;
	ListBox(HWND parent, const std::wstring& text, int width, int height, int x = 0, int y = 0, LONG style = STYLE)
		: BaseSystemWindow(parent, text, width, height, x, y, style) {
	}
	ListBox() : BaseSystemWindow(0, L"", 0, 0, 1, 1, STYLE) {}
	ListBox(ListBox&& other) noexcept : w32oop_ui_window_move_ctor_super(BaseSystemWindow, other),
		w32oop_ui_window_move_ctor_object(onDblclickHandler, other),
		w32oop_ui_window_move_ctor_object(onChangeHandler, other) {
		w32oop_ui_window_move_value(lastSelection, other);
	}
	ListBox& operator=(ListBox&& other) noexcept {
		w32oop_ui_window_move_assignment_start(other);
		w32oop_ui_window_move_super(BaseSystemWindow, other);
		w32oop_ui_window_move_object(onDblclickHandler, other);
		w32oop_ui_window_move_object(onChangeHandler, other);
		w32oop_ui_window_move_value(lastSelection, other);
		w32oop_ui_window_move_assignment_end();
	}
	~ListBox() override {}
protected:
	virtual void setup_event_handlers() override {
		WINDOW_EVENT_HANDLER_SUPER(BaseSystemWindow);
		WINDOW_add_notification_handler(LBN_DBLCLK, onDoubleClicked);
		WINDOW_add_notification_handler(LBN_SELCHANGE, onChange);
		WINDOW_add_notification_handler(LBN_SELCANCEL, onChange);
	}
protected:
	const std::wstring get_class_name() const override {
		return WC_LISTBOXW;
	}
	int lastSelection = -1;
protected:
	CEventHandler onDblclickHandler;
	CEventHandler onChangeHandler;
	void onDoubleClicked(EventData& data) {
		if (onDblclickHandler) {
			data.preventDefault();
			onDblclickHandler(data);
		}
	}
	void onChange(EventData& data) {
		// ignore compare when multiple: users should check by themselves in multiple mode
		if ((multiple() ? true : (lastSelection != getCurrentSelection())) && onChangeHandler) {
			data.preventDefault();
			lastSelection = getCurrentSelection();
			onChangeHandler(data);
		}
	}
public:
	void onDblclick(CEventHandler handler) {
		onDblclickHandler = handler;
	}
	void onChange(CEventHandler handler) {
		onChangeHandler = handler;
	}
public:
	bool multiple() const {
		return get_style() & LBS_MULTIPLESEL;
	}
	void multiple(bool f) {
		if (f) {
			add_style(LBS_MULTIPLESEL | LBS_EXTENDEDSEL);
		}
		else {
			remove_style(LBS_MULTIPLESEL);
			remove_style(LBS_EXTENDEDSEL);
		}
	}
	int getCount() const {
		return ListBox_GetCount(hwnd);
	}
	void resetContent() {
		ListBox_ResetContent(hwnd);
	}
	int getCurrentSelection() const {
		int value = ListBox_GetCurSel(hwnd);
		return value;
	}
	void setCurrentSelection(int index) {
		lastSelection = index;
		ListBox_SetCurSel(hwnd, index);
	}
	void clearSelection() {
		lastSelection = -1;
		// LB_SETCURSEL only moves the focus in a multiple-selection list box,
		// so the selection has to be cleared with LB_SETSEL(-1) there.
		if (multiple()) ListBox_SetSel(hwnd, FALSE, -1);
		else ListBox_SetCurSel(hwnd, -1);
	}
	int addItem(const std::wstring& itemText) {
		return ListBox_AddString(hwnd, itemText.c_str());
	}
	int deleteItem(int index) {
		return ListBox_DeleteString(hwnd, index);
	}
	int insertItem(int index, const std::wstring& itemText) {
		return ListBox_InsertString(hwnd, index, itemText.c_str());
	}
	LRESULT getItemData(int index) const {
		return ListBox_GetItemData(hwnd, index);
	}
	void setItemData(int index, LPARAM data) {
		ListBox_SetItemData(hwnd, index, data);
	}
	std::wstring getItemText(int index) const {
		validate_hwnd();
		int len = ListBox_GetTextLen(hwnd, index);
		if (len <= 0 || len > 1073741824) return {};
		std::wstring buffer(static_cast<size_t>(len) + 1, L'\0');
		int copied = ((int)(DWORD)::SendMessageW((hwnd), 0x0189, (WPARAM)(int)(index), (LPARAM)(LPCTSTR)(buffer.data())));
		if (copied <= 0) return {};
		buffer.resize(static_cast<size_t>(copied));
		return buffer;
	}
	void setItemText(const std::wstring& newText, int index) {
		auto OldData = getItemData(index);
		auto currentSelection = getCurrentSelection();
		ListBox_DeleteString(hwnd, index);
		ListBox_InsertString(hwnd, index, newText.c_str());
		setItemData(index, OldData);
		setCurrentSelection(currentSelection);
	}
	int getSelectionCount() const {
		return ListBox_GetSelCount(hwnd);
	}
	int getTopIndex() const {
		return ListBox_GetTopIndex(hwnd);
	}
	std::vector<int> getSelectedItems() const {
		auto cnt = getSelectionCount();
		if (cnt <= 0) return {};
		// The vector must be sized, not only reserved: ListBox_GetSelItems
		// writes into the buffer and the size is what the caller iterates.
		std::vector<int> value(static_cast<size_t>(cnt));
		int copied = ListBox_GetSelItems(hwnd, cnt, value.data());
		if (copied <= 0) return {};
		value.resize(static_cast<size_t>(copied));
		return value;
	}
	// only use this for multiple box
	void setSelectionState(bool selectionState,int index) {
		ListBox_SetSel(hwnd, selectionState, index);
	}
};

#pragma endregion



#pragma region Common controls. You should InitCommonControlsEx before using them.

class StatusBar : public BaseSystemWindow {
public:
	static const LONG STYLE = WS_CHILD | WS_VISIBLE;
	StatusBar(HWND parent, const std::wstring& text, int width, int height, int x = 0, int y = 0, LONG style = STYLE)
		: BaseSystemWindow(parent, text, width, height, x, y, style) {
	}
	StatusBar() : BaseSystemWindow(0, L"", 0, 0, 1, 1, STYLE) {}
	StatusBar(StatusBar&& other) noexcept : w32oop_ui_window_move_ctor_super(BaseSystemWindow, other) {}
	StatusBar& operator=(StatusBar&& other) noexcept {
		w32oop_ui_window_move_assignment_start(other);
		w32oop_ui_window_move_super(BaseSystemWindow, other);
		w32oop_ui_window_move_assignment_end();
	}
	~StatusBar() override {}
	inline void simple(bool isSimple) {
		send(SB_SIMPLE, isSimple ? TRUE : FALSE, 0);
	}
	std::wstring text() const override {
		return get_text(0);
	}
	std::wstring get_text(int part = 0) const;
	// Note: To operate a non-simple status bar, 
	// please use `set_text(int index, const std::wstring& t)` instead.
	void text(const std::wstring& t) override {
		return set_text(SB_SIMPLEID, t);
	}
	void set_text(int index, const std::wstring& t) {
		send(SB_SETTEXT, LOBYTE(index), (LPARAM)t.c_str());
	}
	void set_parts(int count, int* widths) {
		// SB_SETPARTS takes the right edge of each part, so the values must be
		// scaled by the framework DPI virtualization like any other size.
		// Negative values (e.g. -1 = extend to the right edge) are kept as-is.
		if (count <= 0) return;
		std::vector<int> edges(static_cast<size_t>(count));
		for (int i = 0; i < count; ++i) {
			edges[i] = (widths[i] < 0) ? widths[i] : scaled(widths[i]);
		}
		send(SB_SETPARTS, count, (LPARAM)edges.data());
	}
protected:
	const std::wstring get_class_name() const override {
		return STATUSCLASSNAMEW;
	}
protected:
	virtual void setup_event_handlers() override {
		WINDOW_EVENT_HANDLER_SUPER(BaseSystemWindow);
	}
};

class TrackBar : public BaseSystemWindow {
public:
	static const LONG STYLE = WS_CHILD | WS_VISIBLE | TBS_ENABLESELRANGE | TBS_NOTIFYBEFOREMOVE;
	TrackBar(HWND parent, int width, int height, int x = 0, int y = 0, LONG style = STYLE)
		: BaseSystemWindow(parent, L"", width, height, x, y, style) {
	}
	TrackBar() : BaseSystemWindow(0, L"", 0, 0, 1, 1, STYLE) {}
	TrackBar(TrackBar&& other) noexcept : w32oop_ui_window_move_ctor_super(BaseSystemWindow, other),
		w32oop_ui_window_move_ctor_object(callback, other)
	{}
	TrackBar& operator=(TrackBar&& other) noexcept {
		w32oop_ui_window_move_assignment_start(other);
		w32oop_ui_window_move_super(BaseSystemWindow, other);
		w32oop_ui_window_move_object(callback, other);
		w32oop_ui_window_move_assignment_end();
	}
	~TrackBar() override {}
protected:
	const std::wstring get_class_name() const override {
		return TRACKBAR_CLASSW;
	}
protected:
	virtual void setup_event_handlers() override {
		WINDOW_EVENT_HANDLER_SUPER(BaseSystemWindow);
		WINDOW_add_notification_handler(TRBN_THUMBPOSCHANGING, onThumbPosChanging);
	}
public:
	void set_range(int min_, int max_) {
		send(TBM_SETRANGE, TRUE, MAKELONG(min_, max_));
	}
	void set_page_size(int size) {
		send(TBM_SETPAGESIZE, 0, size);
	}
	void set_selection(int min_, int max_) {
		send(TBM_SETSEL, 0, MAKELONG(min_, max_));
	}
	void set_tick_frequency(int freq) {
		send(TBM_SETTICFREQ, freq, 0);
	}
	void set_ticks(int ticks) {
		send(TBM_SETTIC, 0, ticks);
	}
	void pos(int pos) {
		send(TBM_SETPOS, TRUE, pos);
	}
	int pos() const {
		return (int)send(TBM_GETPOS, 0, 0);
	}
protected:
	std::function<void(EventData&)> callback;
	void onThumbPosChanging(EventData& ev) {
		if (!callback) return;
		callback(ev);
		if (ev.defaultPrevented()) ev.returnValue(TRUE);
	}
public:
	void onbeforechange(std::function<void(EventData&)> callback2) {
		this->callback = callback2;
	}
};

#pragma endregion





#pragma region Dialogs

template<typename T>
concept InputDialog_ValueTypes = std::same_as<T, std::wstring> ||
(std::is_integral_v<T> && (sizeof(T) <= sizeof(std::int64_t)));

// One-time use. To reuse please create new instance
class InputDialog : public Window {
public:
	InputDialog(std::wstring title = L"Input", int width = 320, int height = 145, HWND ownerWindow = NULL) :
		Window(title, (int)(width* w32oop::ui::internal::system_dpi_scale_factor() + 0.5f),
			(int)(height* w32oop::ui::internal::system_dpi_scale_factor() + 0.5f), 0, 0, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_CLIPCHILDREN),
		m_scale((float)w32oop::ui::internal::system_dpi_scale_factor()),
		_logical_width(width), _logical_height(height), parent_window(ownerWindow)
	{
		set_framework_dpi_virtualization(false);
	};
	InputDialog(HWND ownerWindow, std::wstring title = L"Input", int width = 320, int height = 145) :
		Window(title, (int)(width* w32oop::ui::internal::system_dpi_scale_factor() + 0.5f),
			(int)(height* w32oop::ui::internal::system_dpi_scale_factor() + 0.5f), 0, 0, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_CLIPCHILDREN),
		m_scale((float)w32oop::ui::internal::system_dpi_scale_factor()),
		_logical_width(width), _logical_height(height), parent_window(ownerWindow)
	{
		set_framework_dpi_virtualization(false);
	};
	InputDialog(InputDialog&& other) noexcept : w32oop_ui_window_move_ctor_super(Window, other),
		w32oop_ui_window_move_ctor_object(prompt, other),
		w32oop_ui_window_move_ctor_object(editBox, other),
		w32oop_ui_window_move_ctor_object(accept, other),
		w32oop_ui_window_move_ctor_object(reject, other),
		w32oop_ui_window_move_ctor_object(textBuffer, other)
	{
		w32oop_ui_window_move_value(m_scale, other);
		w32oop_ui_window_move_value(_logical_width, other);
		w32oop_ui_window_move_value(_logical_height, other);
		w32oop_ui_window_move_value(parent_window, other);
		w32oop_ui_window_move_object(promptFont, other);
		w32oop_ui_window_move_value(rejected, other);
		w32oop_ui_window_move_value(isActive, other);
		w32oop_ui_window_move_value(isCreated, other);
		w32oop_ui_window_move_value(_usePassmode, other);
	}
	InputDialog& operator=(InputDialog&& other) noexcept {
		w32oop_ui_window_move_assignment_start(other);
		w32oop_ui_window_move_super(Window, other);
		w32oop_ui_window_move_object(prompt, other);
		w32oop_ui_window_move_object(editBox, other);
		w32oop_ui_window_move_object(accept, other);
		w32oop_ui_window_move_object(reject, other);
		w32oop_ui_window_move_object(textBuffer, other);
		w32oop_ui_window_move_value(m_scale, other);
		w32oop_ui_window_move_value(_logical_width, other);
		w32oop_ui_window_move_value(_logical_height, other);
		w32oop_ui_window_move_value(parent_window, other);
		w32oop_ui_window_move_object(promptFont, other);
		w32oop_ui_window_move_value(rejected, other);
		w32oop_ui_window_move_value(isActive, other);
		w32oop_ui_window_move_value(isCreated, other);
		w32oop_ui_window_move_value(_usePassmode, other);
		w32oop_ui_window_move_assignment_end();
	}

protected:
	int s(int n) const {
		return (int)(n * m_scale + 0.5f);
	}
	int l(int n) const {
		return (int)(n / m_scale + 0.5f);
	}

	float m_scale = 1.0f;
	int _logical_width = 320, _logical_height = 145;
	HWND parent_window;
	HWND new_window() override;
	void register_class_if_needed() override;
	std::wstring prompt;
	Edit editBox;
	Button accept, reject;
	w32FontHandle promptFont;
	bool rejected = true;
	bool isActive = true;
	bool isCreated = false;
	bool _usePassmode = false;
	std::wstring textBuffer;

	void onCreated() override;
	void onDestroy() override;
	void paint(EventData& ev);
	void onHittest(EventData& ev);
	void doLayout(EventData& ev);
	void onDpiChanged(EventData& ev);
	void onNcCalcSize(EventData& ev);
	void onNcActivate(EventData& ev);
	void onSetCursor(EventData& ev);
	void onLButtonUp(EventData& ev);

	virtual void setup_event_handlers() override {
		WINDOW_add_handler(WM_PAINT, paint);
		WINDOW_add_handler(WM_NCHITTEST, onHittest);
		WINDOW_add_handler(WM_WINDOWPOSCHANGED, doLayout);
		WINDOW_add_handler(WM_DPICHANGED, onDpiChanged);
		WINDOW_add_handler(WM_NCCALCSIZE, onNcCalcSize);
		WINDOW_add_handler(WM_NCACTIVATE, onNcActivate);
		WINDOW_add_handler(WM_SETCURSOR, onSetCursor);
		WINDOW_add_handler(WM_LBUTTONUP, onLButtonUp);
		WINDOW_add_handler(WM_CLOSE, [this](EventData&) { textBuffer = editBox.text(); });
	}

	inline bool hittest_closeButton(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h) {
		UNREFERENCED_PARAMETER(h);
		return (x >= w - s(40) && x <= w) && (y >= 0 && y < s(40));
	}

	template <InputDialog_ValueTypes value_type>
	std::wstring str(value_type value) {
		if constexpr (std::same_as<value_type, std::wstring>) return value;
		else return std::to_wstring(value);
	}
public:
	template <InputDialog_ValueTypes value_type>
	std::optional<value_type> getInput(std::wstring prompt = L"", std::optional<value_type> default_value = std::nullopt, bool modal = true) {
		// 设置UI元素显示提示和默认值
		if (!isCreated) create();
		if (!prompt.empty()) setPrompt(prompt);
		if (default_value.has_value()) setText(str(default_value.value()));

		show();
		focus();
		editBox.focus();
		HWND ancestor = GetParent(hwnd);
		if (ancestor == hwnd || ancestor == GetDesktopWindow()) ancestor = NULL;
		BOOL previousState = ancestor && IsWindowEnabled(ancestor);
		if (modal && ancestor) EnableWindow(ancestor, FALSE);
		run(this);
		if (modal && ancestor) {
			EnableWindow(ancestor, previousState);
			SetForegroundWindow(ancestor);
		}

		if (rejected) return std::nullopt;
		if constexpr (std::same_as<value_type, std::wstring>) return getText();
		else {
			if (getText().starts_with(L"0x") || getText().starts_with(L"0X"))
				return static_cast<value_type>(wcstoll(getText().c_str() + 2, NULL, 16));
			return static_cast<value_type>(wcstoll(getText().c_str(), NULL, 10));
		}
	}
	std::wstring getPrompt() const { return prompt; }
	void setPrompt(std::wstring new_value) { prompt = new_value; update(); }
	std::wstring getText() const { return textBuffer; }
	void setText(std::wstring text) { editBox.text(text); }
	void setMultiple(bool multiple = true);
	void setAcceptButtonText(std::wstring t) { accept.text(t); }
	void setRejectButtonText(std::wstring t) { reject.text(t); }
	void setButtonsText(std::wstring a, std::wstring b) {
		setAcceptButtonText(a);
		setRejectButtonText(b);
	}
	bool setPasswordMode(bool mode = true) {
		if (editBox.created()) return false;
		_usePassmode = mode;
		return true;
	}
	void setPasswordChar(TCHAR c) { editBox.password_char(c); }
};


#pragma endregion






#undef w32oop_ui_foundation_add_mover
}} // namespace w32oop::ui::foundation

#pragma endregion
