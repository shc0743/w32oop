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

    class HotKeyMixedAppDemo : public Window {
    protected:
        Static text;

    public:
        HotKeyMixedAppDemo(const wstring& title, int width, int height, int x = 0, int y = 0)
            : Window(title, width, height, x, y, WS_OVERLAPPEDWINDOW)
        {
            // Do not initialize the button here
        }
    protected:
        vector<thread> myThreads;
        void onDestroy() override {
            for (auto& t : myThreads) {
                if (t.joinable()) {
                    t.join();
                }
            } 
        }
        void onCreated() override {
            text.set_parent(*this);
            text.create(L"Result will show here...", 400, 30);
            register_hot_key(true, false, false, 'A', [&](HotKeyProcData &data) {
                data.preventDefault();
                // Be careful! When the Hook is globally hooked
                // the event handling will vary!
                // The wParam and lParam's meaning is different.
                if (!data.pKbdStruct) return; 
                // Do not block the hook proc thread
                myThreads.push_back(std::thread([&] {
                    text.text(L"Ctrl+A is pressed Windowed");
                    Sleep(1000);
                    text.text(L"Result will show here...");
                }));
            });
            register_hot_key(true, false, false, 'A', [&](HotKeyProcData &data) {
                data.preventDefault();
                if (!data.pKbdStruct) return;
                // Do not block the hook proc thread
                myThreads.push_back(std::thread([&] {
                    text.text(L"Ctrl+A is pressed Systemwide");
                    Sleep(1000);
                    text.text(L"Result will show here...");
                }));
            }, Window::HotKeyOptions::System);
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
            DrawTextW(hdc, L"Try to press Ctrl+A in your OS and window", -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
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
}

int main() { 
    Window::set_global_option(Window::Option_DebugMode, true);
    // create the application
    MyDemo::HotKeyMixedAppDemo app(L"HotKey Application (Mixed)", 640, 480);
    // create it
    app.create();
    // set the main window
    app.set_main_window();
    // center
    app.center();
    // show it
    app.show();
    // // Set option to allow the hotkey
    // app.set_global_option(Window::Option_EnableHotkey, true);
    // app.set_global_option(Window::Option_EnableGlobalHotkey, true);
    // Not necessary, because we *created* window **BEFORE** calling Window::run()
    // However, if you want to create the window after calling Window::run(), you need to set
    // the option manually.
    // message loop
    return Window::run();
}