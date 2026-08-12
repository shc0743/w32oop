#include "./TrayIcon.hpp"

UINT w32oop::ui::TrayIcon::WM_TaskbarCreated;

w32oop::ui::TrayIcon::TrayIcon() {
	if (!WM_TaskbarCreated) {
		WM_TaskbarCreated = RegisterWindowMessage(TEXT("TaskbarCreated"));
	}

	memset(&nid, 0, sizeof(nid));

	win.create(); // 创建虚拟窗口

	nid.cbSize = sizeof(nid);
	nid.hWnd = win.hwnd; // 关联到虚拟窗口
	nid.uID = 1; // 默认ID
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = WM_USER + 1; // 自定义消息
	nid.uVersion = NOTIFYICON_VERSION_4;

	add(); // 添加托盘图标

	win.addEventListener(WM_TaskbarCreated, [this](EventData& ev) {
		_update();
	});
	addEventListener(nid.uCallbackMessage, [this](EventData& ev) {
		this->handle_event(ev); // 处理托盘图标事件
	});
}

w32oop::ui::TrayIcon::~TrayIcon() {
	remove(); // 删除托盘图标
}

void w32oop::ui::TrayIcon::handle_event(EventData& ev) {
	if (ev.message == nid.uCallbackMessage) {
		// 处理托盘图标的点击事件
		if (ev.lParam == WM_RBUTTONUP && pMenu) {
			ev.preventDefault(); // 阻止默认行为

			pMenu->pop(); // 显示菜单
			PostMessage(win.hwnd, WM_NULL, 0, 0);

			return;
		}
		if (ev.lParam == WM_LBUTTONUP) {
			if (onClickHandler) {
				onClickHandler(ev); // 调用点击事件处理器
			}
			else {
				ev.preventDefault(); // 阻止默认行为
			}
			return;
		}
		if (ev.lParam == WM_LBUTTONDBLCLK) {
			if (onDblclickHandler) {
				onDblclickHandler(ev); // 调用点击事件处理器
			}
			else {
				ev.preventDefault(); // 阻止默认行为
			}
			return;
		}
		if (ev.lParam == NIN_BALLOONUSERCLICK) {
			if (onBalloonClickHandler) {
				onBalloonClickHandler(ev); // 调用通知点击事件处理器
			}
			else {
				ev.preventDefault(); // 阻止默认行为
			}
			return;
		}
	}
}

void w32oop::ui::TrayIcon::showNotification(
	const std::wstring& title, 
	const std::wstring& message,
	UINT uFlags,
	UINT uTimeout
) {
	nid.uFlags |= NIF_INFO; // 设置通知标志
	wcscpy_s(nid.szInfo, message.c_str());
	wcscpy_s(nid.szInfoTitle, title.c_str());
	nid.dwInfoFlags = uFlags;
	nid.uTimeout = uTimeout;
	Shell_NotifyIcon(NIM_MODIFY, &nid); // 修改托盘图标以显示通知
	nid.uFlags &= ~NIF_INFO; // 清除通知标志以避免重复显示
}
