// w32oop Framework Example
// 
// : 04.customizing-window-class/demo.cpp
// This example demonstrates how to customize the window class.

#include <w32use.hpp>

#pragma comment(linker, "\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;
class VeryVeryVeryLongWindow1 : public Window {
public:
    VeryVeryVeryLongWindow1() : Window(L"VeryVeryVeryLongWindow1: Please check with Spy++! (It might be: `class VeryVeryVeryLongWindow1#(C++ Window):.?AVVeryVeryVeryLongWindow1@@#Window`)", 480, 320, 0, 0, WS_OVERLAPPEDWINDOW) {}
    // Recommended to generate class name automatically
protected:
    virtual void setup_event_handlers() override {}
};
class VeryVeryVeryLongWindow2 : public Window {
public:
    VeryVeryVeryLongWindow2() : Window(L"VeryVeryVeryLongWindow2: Please check with Spy++!", 480, 320, 0, 0, WS_OVERLAPPEDWINDOW) {}
    // But you can also customize the class name
    virtual const wstring get_class_name() const override {
        return L"MyShortWndClassName";
    }
protected:
    virtual void setup_event_handlers() override {}
};

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR pCmdLine,
    _In_ int nCmdShow
) {
    Window* app = new VeryVeryVeryLongWindow1();
    app->create();
    app->set_main_window();
    app->center();
    app->show(nCmdShow);
    app->run();
    delete app;

    app = new VeryVeryVeryLongWindow2();
    app->create();
    app->set_main_window();
    app->center();
    app->show(nCmdShow);
    app->run();
    delete app;

    return 0;
}