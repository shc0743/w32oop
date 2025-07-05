// compile command: cl /EHsc demo.cpp /std:c++20
#define UNICODE 1
#define _UNICODE 1
#include <w32use.hpp>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
// #pragma comment(linker, "/entry:mainCRTStartup /subsystem:windows")

using namespace std;

namespace MyDemo {

    class HotKeyAppDemo : public Window {
    protected:

    public:
        HotKeyAppDemo(const wstring& title, int width, int height, int x = 0, int y = 0)
            : Window(title, width, height, x, y, WS_OVERLAPPEDWINDOW)
        {
            // Do not initialize the button here
        }
        ~HotKeyAppDemo() override {
        }
    protected:
        void onCreated() override {
            register_hot_key(true, true, false, 'I', [&](HotKeyProcData &data) {
                data.preventDefault();
                MessageBoxW(hwnd, L"Ctrl+Alt+I is pressed", L"Notification", MB_OK);
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
            DrawTextW(hdc, L"Try to press Ctrl+Alt+I in the window", -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
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
    MyDemo::HotKeyAppDemo app(L"HotKey Application", 640, 480);
    // create it
    app.create();
    // set the main window
    app.set_main_window();
    // center
    app.center();
    // show it
    app.show();
    // Set option to allow the hotkey
    app.set_global_option(Window::Option_EnableHotkey, true);
    // app.set_global_option(Window::Option_EnableGlobalHotkey, true);
    // message loop
    return Window::run();
}