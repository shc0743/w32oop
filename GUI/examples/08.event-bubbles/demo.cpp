// compile command: cl /EHsc demo.cpp /std:c++20
#define UNICODE 1
#define _UNICODE 1
#include "../../Window.hpp"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(linker, "/entry:mainCRTStartup /subsystem:windows")

using namespace w32oop;
using namespace w32oop::foundation;

namespace MyDemo {
	class EventBubbleDemo : public Window {
	protected:
		Button btn1, btn2, btn3;

	public:
		EventBubbleDemo(const wstring& title, int width, int height, int x = 0, int y = 0)
			: Window(title, width, height, x, y, WS_OVERLAPPEDWINDOW)
		{}
	protected:
		void onCreated() override {
			btn1.set_parent(this);
			btn1.create(L"Button1", 60, 30, 10, 10);
            btn2.set_parent(this);
            btn2.create(L"Button2", 60, 30, 10, 50);
			btn3.set_parent(this);
            btn3.create(L"Button3", 60, 30, 10, 90);

			// Bubble:
			addEventListener(WINDOW_NOTIFICATION_CODES + BN_CLICKED, [this](EventData& event) {
				if (!dynamic_cast<Button*>(event.source())) return; // check if it's a button
				MessageBoxW(hwnd, (L"Button: " + event.source()->text()).c_str(), L"Bubbled to Parent Window", MB_ICONINFORMATION);
			});

			// Button1: Doesn't handle; the event will bubble to the parent window.

			// Button2: Handles the event *without* calling `stopPropagation()`.
			//  The event will bubble to the parent window.
			btn2.onClick([this](EventData& event) {
				MessageBoxW(hwnd, L"Button2 Handler", L"Button2", MB_ICONINFORMATION);
			});

			// Button3: Handles the event *with* `stopPropagation()`.
			//  The event will **NOT** bubble to the parent window.
			btn3.onClick([this](EventData& event) {
				MessageBoxW(hwnd, L"Button3 Handler", L"Button3", MB_ICONINFORMATION);
				event.stopPropagation();
			});
		}
		void onClicked(EventData&) {
			MessageBoxW(hwnd, L"Window clicked!", L"Notification", MB_OK);
		}
	private:
		virtual void setup_event_handlers() override {
			WINDOW_add_handler(WM_LBUTTONDOWN, onClicked);
		}
	};

	int main2() {
		// create the application
		EventBubbleDemo app(L"Event Bubble Demo", 640, 480);
		// create it
		app.create();
		// set the main window
		app.set_main_window();
		// center
		app.center();
		// show it
		app.show();
		// message loop
		return Window::run();
	}
}

int main() { 
	try {
		Window::set_global_option(Window::Option_DebugMode, 1);
		int ret = MyDemo::main2();
		return ret;
	}
	catch (const std::exception& e) {
		MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
		return -1;
	}
}