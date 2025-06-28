// Shows the dynamic event-handler.

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
    class AppThatPreventsClose : public Window {
    protected:
        Button myButton;
        Static myResult;

    public:
        AppThatPreventsClose(const wstring& title, int width, int height, int x = 0, int y = 0)
            : Window(title, width, height, x, y, WS_OVERLAPPEDWINDOW)
        {
        }
    protected:
        std::function<void(EventData &)> preventer;
        bool flag = false;
        void onCreated() override {
            myButton.set_parent(*this);
            myButton.create(L"Toggle exit allowance", 150, 30, 10, 10);
            // set event handler
            preventer = [this](EventData& data) {
                data.preventDefault();
            };
            myButton.onClick([&](EventData& data) {
                if (flag) {
                    this->removeEventListener(WM_CLOSE, preventer);
                    this->removeEventListener(WM_QUERYENDSESSION, preventer);
                    this->removeEventListener(WM_ENDSESSION, preventer);
                    myResult.text(L"Closable now!");
                } else {
                    this->addEventListener(WM_CLOSE, preventer);
                    this->addEventListener(WM_QUERYENDSESSION, preventer);
                    this->addEventListener(WM_ENDSESSION, preventer);
                    myResult.text(L"Preventing close!");
                }
                flag = !flag;
            });
            // create the static control
            myResult.set_parent(*this);
            myResult.create(L"Closable!", 300, 30, 10, 50);
        }
    private:
        void onPaint(EventData& data) {
            data.preventDefault();

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            Rectangle(hdc, 100, 200, 300, 300); // (left, top, right, bottom)
            EndPaint(hwnd, &ps);
        }
        virtual void setup_event_handlers() override {
            WINDOW_add_handler(WM_PAINT, onPaint);
        }
    };

    int main2() {
        // create the application
        AppThatPreventsClose app(L"Preventing Close", 640, 480);
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
        int ret = MyDemo::main2();
        return ret;
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}