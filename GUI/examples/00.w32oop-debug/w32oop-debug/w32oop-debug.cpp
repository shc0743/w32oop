#include "../../../Window.hpp"
#include <CommCtrl.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(linker, "/entry:mainCRTStartup /subsystem:windows")

using namespace w32oop;
using namespace w32oop::foundation;

class LibDebug : public Window {
protected:
	Static text;
	Button btn;
	Edit edit;
	Button newWindow;
public:
	LibDebug(const wstring& title, int width, int height, int x = 0, int y = 0)
		: Window(title, width, height, x, y, WS_OVERLAPPEDWINDOW)
	{
		// Do not initialize the button here
		if (!nextKey) nextKey = 'A';
	}
	static char nextKey;
protected:
	char key = 0;
	wchar_t wk = 0;
	vector<thread> myThreads;
	void onDestroy() override {
		for (auto& t : myThreads) t.join();
	}
	void onCreated() override {
		text.set_parent(*this);
		text.create(L"Result will show here...", 400, 30);
		key = nextKey; wk = key;
		++nextKey;
		register_hot_key(true, false, false, key, [&](HotKeyProcData& data) {
			data.preventDefault();
			//if (!data.pKbdStruct) return;
			// Do not block the hook proc thread
			myThreads.push_back(std::thread([this] {
				text.text(L"Ctrl+"s + wk + L" is pressed Windowed");
				printf("Ctrl+%c is pressed Windowed\n", key);
				fflush(stdout);
				Sleep(1000);
				// 注意：窗口可能已经销毁！所以我们需要集中管理这些线程。（不能detach）
				text.text(L"Result will show here...");
			}));
		});
		register_hot_key(true, false, false, key, [&](HotKeyProcData& data) {
			data.preventDefault();
			//if (!data.pKbdStruct) return;
			// Do not block the hook proc thread
			myThreads.push_back(std::thread([this] {
				text.text(L"Ctrl+"s + wk + L" is pressed Systemwide");
				printf("Ctrl+%c is pressed Systemwide\n", key);
				fflush(stdout);
				Sleep(1000);
				// 注意：窗口可能已经销毁！所以我们需要集中管理这些线程。（不能detach）
				text.text(L"Result will show here...");
			}));
		 }, Window::HotKeyOptions::System);

		btn.set_parent(this);
		btn.create(L"Remove hotkey", 200, 30, 10, 30, Button::STYLE | BS_SPLITBUTTON);
		btn.onClick([=](EventData& event) {
			remove_hot_key(true, false, false, key);
			btn.text(L"Removed");
			btn.disable();
		});
		btn.on(BCN_DROPDOWN, [&](EventData& event) {
			text.text(L"You clicked the dropdown button");
		});

		edit.set_parent(this);
		edit.create(L"", 200, 30, 10, 60);
		edit.onChange([&](EventData& event) {
			 text.text(L"You edited!");
		});

		newWindow.set_parent(this);
		newWindow.create(L"Create New Window", 250, 30, 10, 90);
		newWindow.onClick([&](EventData& event) {
			thread([&] {
				try {
					LibDebug app(L"New Window", 640, 480);
					app.create();
					app.center();
					app.show();
					app.set_main_window();
					//app.set_global_option(Window::Option_EnableGlobalHotkey, false);
					return app.run();
				}
				catch (exception& exc) {
					MessageBoxA(nullptr, exc.what(), "Error", MB_ICONERROR);
					return 1;
				}
			}).detach();
		});
	}
	void onPaint(EventData& event) {
		event.preventDefault();
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
		HFONT hOldFont = nullptr;
		if (hFont) {
			hOldFont = (HFONT)SelectObject(hdc, hFont);
		}
		RECT rect;
		GetClientRect(hwnd, &rect);
		DrawTextW(hdc, (L"Try to press Ctrl+"s + wk + L" in your OS and window").c_str(), -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		if (hOldFont) {
			SelectObject(hdc, hOldFont);
		}
		EndPaint(hwnd, &ps);
	}
	void onKeydown(EventData& event) {
		WPARAM vk = event.wParam;
		if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9')) {
			MessageBoxW(hwnd, L"You pressed a key!", L"Notification", MB_OK);
		}
	}
private:
	virtual void setup_event_handlers() override {
		WINDOW_add_handler(WM_PAINT, onPaint);
		WINDOW_add_handler(WM_KEYDOWN, onKeydown);
	}
};
char LibDebug::nextKey;

static LONG WINAPI myhandler(
	_In_ struct _EXCEPTION_POINTERS* ExceptionInfo
) {
	MessageBoxW(nullptr, (L"Exception! " + to_wstring(ExceptionInfo->ExceptionRecord->ExceptionCode)).c_str(), L"Error", MB_ICONERROR);
	return EXCEPTION_EXECUTE_HANDLER;
}

int main() {
	SetUnhandledExceptionFilter(myhandler);
	Window::set_global_option(Window::Option_DebugMode, true);
	LibDebug app(L"Debug Application [Main Thread]", 640, 480);
	app.create();
	app.set_main_window();
	app.center();
	app.show();
	return Window::run();
}