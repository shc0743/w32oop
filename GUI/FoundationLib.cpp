#include "FoundationLib.hpp"
using namespace w32oop;
std::atomic<unsigned long long> w32oop::ui::BaseSystemWindow::ctlid_generator{0x100};
// IsDialogMessage might handle Esc and so differently so we need to use larger Control ID to avoid conflict




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
		scaled(setup_info->x), scaled(setup_info->y),
		scaled(setup_info->width), scaled(setup_info->height),
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
		s(20), 0, 0, 0,
		FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Consolas"
	);

	// 布局控件
	isCreated = true;
	post(WM_SIZE);
}

void w32oop::ui::foundation::InputDialog::onDestroy() {
	// 在这里销毁 GDI 对象。
	DeleteObject(promptFont);
	isCreated = false;
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
	// 在 (1, 1)--(w - 1, s(40)) 绘制标题栏。背景颜色：
	// 活动: RGB(204, 213, 240)
	// 非活动：RGB(204, 204, 204)
	{
		HBRUSH titleBarBrush = CreateSolidBrush(isActive ?
			RGB(204, 213, 240) : RGB(204, 204, 204));
		RECT titleRect = { 1, 1, w - 1, s(40) };
		FillRect(dc, &titleRect, titleBarBrush);
		DeleteObject(titleBarBrush);
		HBRUSH closeBtnBrush = CreateSolidBrush(RGB(255, 0, 0));
		RECT cbRect = { w - s(40), 1, w - 1, s(40) };
		FillRect(dc, &cbRect, closeBtnBrush);
		DeleteObject(closeBtnBrush);
	}
	{
		// 保存原始DC状态
		HFONT oldFont = (HFONT)SelectObject(dc, promptFont);
		COLORREF oldColor = SetTextColor(dc, RGB(0, 0, 0));  // 黑色文本
		int oldBkMode = SetBkMode(dc, TRANSPARENT);          // 透明背景

		RECT textRect = { s(10), 1, w - s(50), s(39) };
		DrawTextW(dc, text().c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		textRect = { s(10), s(45), w - s(20), s(65) };
		DrawTextW(dc, prompt.c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

		// 恢复DC状态
		SetBkMode(dc, oldBkMode);
		SetTextColor(dc, oldColor);
		SelectObject(dc, oldFont);

		oldColor = SetTextColor(dc, RGB(0xFF, 0xFF, 0xFF));
		oldBkMode = SetBkMode(dc, TRANSPARENT);
		textRect = { w - s(40), 1, w - 1, s(40) };
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
	if ((x >= 0 && x <= s(10)) && (y >= 0 && y <= s(10))) ret = HTTOPLEFT;
	else if ((x >= w - s(10) && x <= w) && (y >= 0 && y <= s(10))) ret = HTTOPRIGHT;
	else if ((x >= 0 && x <= s(10)) && (y >= h - s(10) && y <= h)) ret = HTBOTTOMLEFT;
	else if ((x >= w - s(10) && x <= w) && (y >= h - s(10) && y <= h)) ret = HTBOTTOMRIGHT;
	else if (x >= 0 && x <= w && (y >= 0 && y <= s(10))) ret = HTTOP;
	else if (x >= 0 && x <= w && (y >= h - s(10) && y <= h)) ret = HTBOTTOM;
	else if ((x >= 0 && x <= s(10)) && (y >= 0 && y <= h)) ret = HTLEFT;
	else if ((x >= w - s(10) && x <= w) && (y >= 0 && y <= h)) ret = HTRIGHT;
	else if (hittest_closeButton(x, y, w, h)) ret = HTCLIENT;
	else if (y <= s(40)) ret = HTCAPTION;
	else ret = HTCLIENT;
	ev.returnValue(ret);
}

void w32oop::ui::foundation::InputDialog::doLayout(EventData& ev) {
	if (!editBox || !accept || !reject) return;

	// 窗口级虚拟化已关闭：GetClientRect 返回物理客户区。
	// 固定偏移量必须按 m_scale 缩放，否则控件与窗口（以及 paint 中
	// 使用 s() 的提示文字/标题栏）在 DPI > 96 时比例错乱、互相重叠。
	RECT rc{}; GetClientRect(hwnd, &rc);
	auto w = rc.right - rc.left, h = rc.bottom - rc.top;

	editBox.resize(s(10), s(70), w - s(20), h - s(120));
	accept.resize(s(10), h - s(40), (w - s(30)) / 2, s(30));
	reject.resize(s(20) + ((w - s(30)) / 2), h - s(40), (w - s(30)) / 2, s(30));
}

void w32oop::ui::foundation::InputDialog::onDpiChanged(EventData& ev) {
	UINT newDpi = HIWORD(ev.wParam);
	float new_scale = (float)newDpi / 96.0f;
	if (new_scale == m_scale) return;
	m_scale = new_scale;

	// 自绘字体大小随新缩放系数重建（onDestroy 里统一销毁）
	DeleteObject(promptFont);
	promptFont = CreateFontW(s(20), 0, 0, 0,
		FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Consolas");

	const RECT* suggested = reinterpret_cast<const RECT*>(ev.lParam);
	if (suggested) {
		SetWindowPos(hwnd, nullptr,
			suggested->left, suggested->top,
			suggested->right - suggested->left,
			suggested->bottom - suggested->top,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
	update();
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

void w32oop::ui::foundation::InputDialog::onSetCursor(EventData& ev) {
	if (LOWORD(ev.lParam) == HTCLIENT) {
		RECT rc{}; GetClientRect(hwnd, &rc);
		auto w = rc.right - rc.left, h = rc.bottom - rc.top;
		POINT pt{}; GetCursorPos(&pt);
		ScreenToClient(hwnd, &pt);
		auto x = pt.x, y = pt.y;
		if (hittest_closeButton(x, y, w, h)) {
			ev.returnValue(TRUE);
			static HCURSOR hHand = NULL;
			if (!hHand) hHand = LoadCursorW(NULL, IDC_HAND);
			SetCursor(hHand);
		}
	}
}

void w32oop::ui::foundation::InputDialog::onLButtonUp(EventData& ev) {
	ev.preventDefault();
	RECT rc{}; GetWindowRect(hwnd, &rc);
	auto w = rc.right - rc.left, h = rc.bottom - rc.top;
	POINT pt{ GET_X_LPARAM(ev.lParam), GET_Y_LPARAM(ev.lParam) };
	auto x = pt.x, y = pt.y;
	if (hittest_closeButton(x, y, w, h)) {
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

