// w32oop Framework Example
// 
// : 01.first-app/demo.cpp
// This example demonstrates a simple GUI application using the w32oop framework.

#include <w32use.hpp>
#include <commctrl.h>

#pragma comment(linker, "\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;
class MyFirstApp : public Window {
public:
    MyFirstApp() : Window(L"My First App", 480, 320, 0, 0, WS_OVERLAPPEDWINDOW) {}
protected:
    void onClicked(EventData& event) {
        int i = 0;
        TaskDialog(hwnd, NULL, L"My First App MessageBox",
            L"You did it!", L"The window was clicked.", TDCBF_CANCEL_BUTTON | TDCBF_OK_BUTTON,
            TD_INFORMATION_ICON, &i);
    }
    void onPaint(EventData& event) {
        event.preventDefault();
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        TextOutW(hdc, 10, 10, L"You did it!", 11);
        EndPaint(hwnd, &ps);
    }
    virtual void setup_event_handlers() override {
        WINDOW_add_handler(WM_LBUTTONDOWN, onClicked);
        WINDOW_add_handler(WM_PAINT, onPaint);
    }
};

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR pCmdLine,
    _In_ int nCmdShow
) {
    // Optional: Enabling debug mode
    Window::set_global_option(Window::GlobalOptions::Option_DebugMode, true);
    // Running your app
    MyFirstApp app;
    app.create();
    app.set_main_window();
    app.center();
    app.show(nCmdShow);
    return app.run();
}