// w32oop Framework Example
// 
// : 03.handling-events/demo.cpp
// This example demonstrates how to handle events in a GUI application using the w32oop framework.

#include <w32use.hpp>
#include <commctrl.h>

#pragma comment(linker, "\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;
class AppThatAllowsConfirmationOnClose : public Window {
public:
    AppThatAllowsConfirmationOnClose() : Window(L"Edit Box", 480, 320, 0, 0, WS_OVERLAPPEDWINDOW) {}
protected:
    Edit edit;
    Button save;
    Static text;
    bool unsaved = false;
    void onCreated() {
        edit.set_parent(this);
        edit.create(L"Write something here...", 300, 200, 10, 10);
        save.set_parent(this);
        save.create(L"Save", 100, 30, 10, 240);
        text.set_parent(this);
        text.create(L"nothing to be saved", 300, 30, 120, 240);

        edit.onChange([this](EventData& event) {
            unsaved = true;
            text.text(L"Unsaved changes detected");
        });
        save.onClick([this](EventData& event) {
            // add your real save logic here
            unsaved = false;
            text.text(L"Changes saved successfully");
        });
    }
    void onClose(EventData& event) {
        if (unsaved) {
            int result = MessageBoxW(
                hwnd,
                L"You have unsaved changes. Do you really want to close?",
                L"Confirm Close",
                MB_YESNO | MB_ICONQUESTION
            );
            if (result == IDNO) {
                event.preventDefault(); // Prevent the window from closing
            }
        }
    }
    virtual void setup_event_handlers() override {
        WINDOW_add_handler(WM_CLOSE, onClose);
    }
};

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR pCmdLine,
    _In_ int nCmdShow
) {
    AppThatAllowsConfirmationOnClose app;
    app.create();
    app.set_main_window();
    app.center();
    app.show(nCmdShow);
    return app.run();
}