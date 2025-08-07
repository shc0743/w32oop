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
#include "./Menu.hpp"


namespace w32oop::ui {
	class TrayIcon : public w32TrayIconObject {
	protected:
		NOTIFYICONDATAW nid;
		
		class VirtualWindow : public Window {
		protected:
			VirtualWindow() : Window(L"Tray Icon Window", 1, 1, 0, 0, WS_POPUP) {}
			virtual void setup_event_handlers() override {
				addEventListener(WM_CLOSE, [this](EventData& ev) {
					ev.preventDefault(); // 阻止关闭事件
				});
			}
			friend class TrayIcon;
		};
		VirtualWindow win;

	public:
		TrayIcon() {
			memset(&nid, 0, sizeof(nid));

			win.create(); // 创建虚拟窗口

			nid.cbSize = sizeof(nid);
			nid.hWnd = win.hwnd; // 关联到虚拟窗口
			nid.uID = 1; // 默认ID
			nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
			nid.uCallbackMessage = WM_USER + 1; // 自定义消息
			nid.uVersion = NOTIFYICON_VERSION_4;

			add(); // 添加托盘图标

			addEventListener(nid.uCallbackMessage, [this](EventData& ev) {
				this->handle_event(ev); // 处理托盘图标事件
			});
		}
		~TrayIcon() {
			remove(); // 删除托盘图标
		}

		void add() {
			Shell_NotifyIcon(NIM_ADD, &nid); // 添加托盘图标
		}
		void remove() {
			Shell_NotifyIcon(NIM_DELETE, &nid); // 删除托盘图标
		}

		void addEventListener(UINT message, function<void(EventData&)> handler) {
			win.addEventListener(message, handler);
		}
		void removeEventListener(UINT message) {
			win.removeEventListener(message);
		}
		void removeEventListener(UINT message, function<void(EventData&)> handler) {
			win.removeEventListener(message, handler);
		}
		void setIcon(HICON icon) {
			nid.hIcon = icon;
			Shell_NotifyIcon(NIM_MODIFY, &nid); // 修改托盘图标
		}
		void setTooltip(const std::wstring& tooltip) {
			wcscpy_s(nid.szTip, tooltip.c_str());
			Shell_NotifyIcon(NIM_MODIFY, &nid); // 修改托盘提示文本
		}
		HWND getWindowHandle() const {
			return win.hwnd; // 返回虚拟窗口的句柄
		}
		
		NOTIFYICONDATAW& getNID() {
			return nid; // 返回NOTIFYICONDATAW结构体
		}

		// 自定义行为
	protected:
		void handle_event(EventData& ev);
	protected:
		Menu* pMenu = nullptr; // 可选的菜单指针
	public:
		Menu* getMenu() const {
			return pMenu; // 获取菜单指针
		}
		void setMenu(Menu* menu) {
			pMenu = menu; // 设置菜单指针
			Shell_NotifyIcon(NIM_MODIFY, &nid); // 更新托盘图标
		}
	protected:
		function<void(EventData&)> onClickHandler = nullptr; // 点击事件处理器
		function<void(EventData&)> onDblclickHandler = nullptr; // 双击事件处理器
		function<void(EventData&)> onBalloonClickHandler = nullptr; // 通知点击事件处理器
	public:
		function<void(EventData&)>& onClick() {
			return onClickHandler; // 获取点击事件处理器
		}
		void onClick(function<void(EventData&)> handler) {
			onClickHandler = handler; // 设置点击事件处理器
		}
		function<void(EventData&)>& onDblclick() {
			return onDblclickHandler; // 获取双击事件处理器
		}
		void onDblclick(function<void(EventData&)> handler) {
			onDblclickHandler = handler; // 设置双击事件处理器
		}
		function<void(EventData&)>& onBalloonClick() {
			return onBalloonClickHandler; // 获取通知点击事件处理器
		}
		void onBalloonClick(function<void(EventData&)> handler) {
			onBalloonClickHandler = handler; // 设置通知点击事件处理器
		}

	public:
		void showNotification(
			const std::wstring& title,
			const std::wstring& message,
			UINT uFlags = NIIF_INFO,
			UINT uTimeout = 10000
		);
	};
}
