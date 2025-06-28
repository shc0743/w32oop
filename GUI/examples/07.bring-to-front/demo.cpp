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

    class HotKeyB2FDemo : public Window {
    protected:
        Static text;

    public:
        HotKeyB2FDemo(const wstring& title, int width, int height, int x = 0, int y = 0)
            : Window(title, width, height, x, y, WS_OVERLAPPEDWINDOW)
        {
            // Do not initialize the button here
        }
    protected:
        void onCreated() override {
            text.set_parent(*this);
            text.create(L"Result will show here...", 400, 30);
            register_hot_key(true, true, false, 'G', [&](HotKeyProcData &data) {
                data.preventDefault();
                if (!data.pKbdStruct) return;
                // Do not block the hook proc thread! (Especially when the Hook is globally hooked)
                std::thread([&] {
                    text.text(L"Ctrl+Alt+G is pressed!");
                    post(WM_USER + 2);
                    Sleep(1000);
                    text.text(L"Result will show here...");
                }).detach();
            }, Window::HotKeyOptions::System);
        }
        void onPaint(EventData& event) {
            event.preventDefault();
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HFONT hFont = get_font();
            HFONT hOldFont = nullptr;
            if (hFont) {
                hOldFont = (HFONT)SelectObject(hdc, hFont);
            }
            RECT rect;
            GetClientRect(hwnd, &rect);
            DrawTextW(hdc, L"Try to press Ctrl+Alt+G in your OS", -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
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
            WINDOW_add_handler(WM_USER + 2, [this](EventData &) {
                force_focus();
            });
        }
    };
}

int main() { 
    Window::set_global_option(Window::Option_DebugMode, true);
    // create the application
    MyDemo::HotKeyB2FDemo app(L"HotKey Application (Bring itself to front)", 640, 480);
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