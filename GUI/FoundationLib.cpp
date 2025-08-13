#include "FoundationLib.hpp"
using namespace w32oop;
std::atomic<unsigned long long> w32oop::ui::BaseSystemWindow::ctlid_generator;




bool w32oop::ui::BaseSystemWindow::class_registered() const {
	return true;
}


HWND w32oop::ui::BaseSystemWindow::new_window() {
	auto cls = get_class_name();
	HWND w = CreateWindowExW(
		setup_info->styleEx,
		cls.c_str(),
		setup_info->title.c_str(),
		setup_info->style,
		setup_info->x, setup_info->y,
		setup_info->width, setup_info->height,
		parent_window, // 必须提供，否则会失败（逆天Windows控件库。。。）并且不可以变化，否则丢消息。。。
		(HMENU)(LONG_PTR)(ctlid), GetModuleHandle(NULL), nullptr
	);
	if (!w) return NULL;
	SetWindowLongPtr(w, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	return w;
}

LRESULT w32oop::ui::BaseSystemWindow::default_handler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (!old_wndproc) return Window::default_handler(hwnd, message, wParam, lParam);
	return CallWindowProcW(old_wndproc, hwnd, message, wParam, lParam);
}

wstring w32oop::ui::foundation::StatusBar::get_text(int part) const {
	int len = LOWORD(send(SB_GETTEXTLENGTH, part, 0));
	if (!len || len < 0 || len > 32768) return wstring();
	auto buffer = std::make_unique<WCHAR[]>(static_cast<size_t>(len) + 1);
	send(SB_GETTEXTW, part, (LPARAM)buffer.get());
	return buffer.get();
}

void w32oop::ui::foundation::InputDialog::onCreated() {
	center(); set_topmost(true);

	editBox = Edit(hwnd, L"", 1, 1); editBox.create();
	accept = Button(hwnd, L"OK", 1, 1, 0, 0, 0, Button::STYLE | BS_DEFPUSHBUTTON);
	reject = Button(hwnd, L"Cancel", 1, 1);
	accept.create(); reject.create();
	accept.onClick([this](EventData&) { rejected = false; close(); });
	reject.onClick([this](EventData&) { rejected = true; close(); });

	register_hot_key(false, false, false, VK_RETURN, [this](HotKeyProcData& ev) {
		if (ES_MULTILINE & GetWindowLongPtr(editBox, GWL_STYLE)) return;
		ev.preventDefault();
		rejected = false;
        close();
	}, HotKeyOptions::Windowed);

	register_hot_key(false, false, false, VK_ESCAPE, [this](HotKeyProcData& ev) {
		ev.preventDefault();
		rejected = true;
        close();
	}, HotKeyOptions::Windowed);

	// 在这里创建 paint 中所需的字体。
	promptFont = CreateFontW(
		20, 0, 0, 0,
		FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Consolas"
	);

	// 布局控件
	post(WM_SIZE);
}

void w32oop::ui::foundation::InputDialog::onDestroy() {
	// 在这里销毁 GDI 对象。
	DeleteObject(promptFont);
}

void w32oop::ui::foundation::InputDialog::paint(EventData& ev) {
	ev.preventDefault();
	PAINTSTRUCT ps{};
	HDC dc = BeginPaint(hwnd, &ps);

	RECT rc{}; GetClientRect(hwnd, &rc);
	auto w = rc.right - rc.left, h = rc.bottom - rc.top;
	// 绘制边框。
	SetDCPenColor(dc, 0); // 边框颜色：黑色
	Rectangle(dc, 0, 0, w, h);
	// 在 (1, 1)--(w - 1, 40) 绘制标题栏。背景颜色：
	// 活动: RGB(204, 213, 240)
	// 非活动：RGB(204, 204, 204)
	{
		HBRUSH titleBarBrush = CreateSolidBrush(isActive ?
			RGB(204, 213, 240) : RGB(204, 204, 204));
		RECT titleRect = { 1, 1, w - 1, 40 };
		FillRect(dc, &titleRect, titleBarBrush);
		DeleteObject(titleBarBrush);
		HBRUSH closeBtnBrush = CreateSolidBrush(RGB(255, 0, 0));
		RECT cbRect = { w - 40, 1, w - 1, 40 };
		FillRect(dc, &cbRect, closeBtnBrush);
		DeleteObject(closeBtnBrush);
	}
	{
		// 保存原始DC状态
		HFONT oldFont = (HFONT)SelectObject(dc, promptFont);
		COLORREF oldColor = SetTextColor(dc, RGB(0, 0, 0));  // 黑色文本
		int oldBkMode = SetBkMode(dc, TRANSPARENT);          // 透明背景

		RECT textRect = { 10, 1, w - 20, 39 };
		DrawTextW(dc, text().c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		textRect = { 10, 45, w - 20, 65 };
		DrawTextW(dc, prompt.c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

		// 恢复DC状态
		SetBkMode(dc, oldBkMode);
		SetTextColor(dc, oldColor);
		SelectObject(dc, oldFont);

		oldColor = SetTextColor(dc, RGB(0xFF, 0xFF, 0xFF));
		oldBkMode = SetBkMode(dc, TRANSPARENT);
		textRect = { w - 40, 1, w - 1, 40 };
		DrawTextW(dc, L"x", -1, &textRect, DT_LEFT | DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		SetBkMode(dc, oldBkMode);
		SetTextColor(dc, oldColor);
	}

	EndPaint(hwnd, &ps);
}

void w32oop::ui::foundation::InputDialog::onHittest(EventData& ev) {
	int ret = HTCAPTION;
	RECT rc{}; GetWindowRect(hwnd, &rc);
	auto w = rc.right - rc.left, h = rc.bottom - rc.top;
	POINT pt{ GET_X_LPARAM(ev.lParam), GET_Y_LPARAM(ev.lParam) };
	ScreenToClient(hwnd, &pt);
	auto x = pt.x, y = pt.y;
	if ((x >= 0 && x <= 10) && (y >= 0 && y <= 10)) ret = HTTOPLEFT;
	else if ((x >= w - 10 && x <= w) && (y >= 0 && y <= 10)) ret = HTTOPRIGHT;
	else if ((x >= 0 && x <= 10) && (y >= h - 10 && y <= h)) ret = HTBOTTOMLEFT;
	else if ((x >= w - 10 && x <= w) && (y >= h - 10 && y <= h)) ret = HTBOTTOMRIGHT;
	else if (x >= 0 && x <= w && (y >= 0 && y <= 10)) ret = HTTOP;
	else if (x >= 0 && x <= w && (y >= h - 10 && y <= h)) ret = HTBOTTOM;
	else if ((x >= 0 && x <= 10) && (y >= 0 && y <= h)) ret = HTLEFT;
	else if ((x >= w - 10 && x <= w) && (y >= 0 && y <= h)) ret = HTRIGHT;
	else if ((x >= w - 40 && x <= w) && (y >= 0 && y < 40)) ret = HTCLIENT;
	else if (y <= 40) ret = HTCAPTION;
	else ret = HTCLIENT;
	ev.returnValue(ret);
}

void w32oop::ui::foundation::InputDialog::doLayout(EventData& ev) {
	if (!editBox || !accept || !reject) return;

	RECT rc{}; GetClientRect(hwnd, &rc);
	auto w = rc.right - rc.left, h = rc.bottom - rc.top;

	editBox.resize(10, 70, w - 20, h - 120);
	accept.resize(10, h - 40, (w - 30) / 2, 30);
	reject.resize(20 + ((w - 30) / 2), h - 40, (w - 30) / 2, 30);
}

void w32oop::ui::foundation::InputDialog::onNcCalcSize(EventData& ev) {
	if (!ev.wParam) return;
	ev.returnValue(0);
}

void w32oop::ui::foundation::InputDialog::onNcActivate(EventData& ev) {
	isActive = ev.wParam;
	update();
	ev.returnValue(TRUE);
}

void w32oop::ui::foundation::InputDialog::onLButtonUp(EventData& ev) {
	ev.preventDefault();
	RECT rc{}; GetWindowRect(hwnd, &rc);
	auto w = rc.right - rc.left, h = rc.bottom - rc.top;
	POINT pt{ GET_X_LPARAM(ev.lParam), GET_Y_LPARAM(ev.lParam) };
	auto x = pt.x, y = pt.y;
	if ((x >= w - 40 && x <= w) && (y >= 0 && y < 40)) {
		close();
	}
}

void w32oop::ui::foundation::InputDialog::setMultiple(bool multiple) {
	constexpr auto v = ES_MULTILINE | ES_WANTRETURN | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_HSCROLL | WS_VSCROLL;
	// 由于创建后无法修改样式，只能先销毁再创建
	editBox = Edit(hwnd, L"", 1, 1, 0, 0, Edit::STYLE | (multiple ? v : 0));
	editBox.create();
	post(WM_SIZE); // 更新布局
}

