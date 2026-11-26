// w32oop Framework Example
// 
// : 03.handling-events/demo.cpp
// This example demonstrates how to handle events in a GUI application using the w32oop framework.

#include <w32use.hpp>
#include <algorithm>
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
        edit.set_parent(this); // this is ugly, isn't it? we are also exploring a better API interface to express the parent-child relationship
        edit.create(L"Write something here...", 300, 200, 10, 10, Edit::STYLE | ES_MULTILINE | ES_WANTRETURN);
        save.set_parent(this);
        save.create(L"Save", 100, 30, 10, 240);
        text.set_parent(this);
        text.create(L"nothing to be saved", 300, 30, 120, 240);

        edit.onChange([this](EventData&) {
            unsaved = true;
            text.text(L"Unsaved changes detected");
        });
        save.onClick([this](EventData&) {
            // add your real save logic here
            unsaved = false;
            text.text(L"Changes saved successfully");
        });

        // Microsoft will send a WM_WINDOWPOSCHANGED so dont worry and there is no need to dispatch it manually
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
    void doLayout(EventData&) {
        if (!edit.created() || !save.created() || !text.created()) return;

        RECT rc = client_rect();
        const int width = rc.right - rc.left;
        const int height = rc.bottom - rc.top;
        constexpr int spacing = 10;
        constexpr int buttonWidth = 100;
        constexpr int buttonHeight = 30;
        const int footerY = (std::max)(spacing, height - spacing - buttonHeight);

        // This is the manual-layout example: it is not difficult to notice that it is more complex than using a layout manager
        edit.resize(
            spacing,
            spacing,
            (std::max)(0, width - 2 * spacing),
            (std::max)(0, footerY - 2 * spacing)
        ); // we always recommend you to fully resize the controls in the layout handler (if you are manually implementing this). This
        // is because use only `move_to` does NOT re-calculate the physical pixels (`move_to` uses SWP_NOSIZE). If the user changed the 
        // DPI then the application will be ugly unless you use `resize` to fully resize (so that we automatically calculate physical size).
        save.resize(spacing, footerY, buttonWidth, buttonHeight);
        const int textX = spacing + buttonWidth + spacing;
        text.resize(
            textX,
            footerY,
            (std::max)(0, width - textX - spacing),
            buttonHeight
        );
    }
    virtual void setup_event_handlers() override {
        WINDOW_add_handler(WM_CLOSE, onClose);
        // you can also use WM_SIZE. the difference is small in this example, but
        // if you need to know more about the event, WM_WINDOWPOSCHANGED provides more information.
        WINDOW_add_handler(WM_WINDOWPOSCHANGED, doLayout); 
    }
};

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR pCmdLine,
    _In_ int nCmdShow
) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pCmdLine);
    AppThatAllowsConfirmationOnClose app;
    app.create();
    app.set_main_window();
    app.center();
    app.show(nCmdShow);
    return app.run();
}