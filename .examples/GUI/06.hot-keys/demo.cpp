// w32oop Framework Example
// 
// : 06.hot-keys/demo.cpp
// This example demonstrates how to use hot keys in w32oop framework.

#include <w32use.hpp>
#include <commctrl.h>
#include <vector>
#include <format>
#include <iostream>

#pragma comment(linker, "\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;

class HotKeyDemoWindow : public Window {
public:
    HotKeyDemoWindow(int mode, char key) : Window(L"HotKey Example Window", 320, 160, 0, 0, WS_OVERLAPPEDWINDOW) {
        this->mode = mode;
        this->key = key;
    }
protected:
    int mode;
    char key;

    CheckBox pd, bringToFrontWhenKeyPressed;
    Static text;

    vector<thread> myThreads;
    void onDestroy() override {
        for (auto& t : myThreads) {
            if (t.joinable()) {
                t.join();
            }
        } 
    }

    wstring str;

    void onCreated() {
        text.set_parent(this);
        str = format(L"Trigger: Ctrl+Alt+{}", key);
        text.create(str, 260, 20, 10, 10);

        pd.set_parent(this);
        pd.create(L"Prevent default", 290, 20, 10, 40);
        pd.check();
        bringToFrontWhenKeyPressed.set_parent(this);
        bringToFrontWhenKeyPressed.create(L"Bring to front when hot-key is pressed", 290, 20, 10, 70);

        HotKeyOptions::Scope scope;
        if (mode == 1) {
            scope = HotKeyOptions::Scope::Windowed;
        } else if (mode == 2) {
            scope = HotKeyOptions::Scope::Thread;
        } else if (mode == 3) {
            scope = HotKeyOptions::Scope::System;
        }

        register_hot_key(true, true, false, key, [this](HotKeyProcData &event) {
            if (pd.checked()) event.preventDefault();
            // preventDefault is important in hot-key processing!
            // If preventDefault is not called, the hot-key will be processed by other handlers or applications.
            // You can uncheck the "Prevent default" checkbox to see the difference.

            // ------

            text.text(L"Hot key is pressed");
            myThreads.push_back(std::thread([&] {
                Sleep(1000);
                text.text(str);
            }));

            if (bringToFrontWhenKeyPressed.checked()) force_focus();
        }, scope);
    }

    virtual void setup_event_handlers() override {
        WINDOW_add_handler(WM_KEYDOWN, [this](EventData& event) {
            WPARAM vk = event.wParam;
            if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9'))
            myThreads.push_back(std::thread([&] {
                text.text(L"Some key (not hotkey) is pressed");
                Sleep(1000);
                text.text(str);
            }));
        });
    }

public:
    bool IsAlive() {
        return is_alive();
    }
};

class HotKeyDemoMainApplication : public Window {
public:
    HotKeyDemoMainApplication() : Window(L"HotKey App", 320, 180, 0, 0, WS_OVERLAPPEDWINDOW) {}
protected:
    CheckBox cWindow, cThread, cSystem;
    Button btnOpenWnd;
    vector<HotKeyDemoWindow *> windows;
    char cchNextKey = 'A';
    void onCreated() {
        cWindow.set_parent(this);
        cWindow.create(L"Window-level HotKey", 280, 20, 10, 10);
        cThread.set_parent(this);
        cThread.create(L"Thread-level HotKey", 280, 20, 10, 40);
        cSystem.set_parent(this);
        cSystem.create(L"System-level HotKey", 280, 20, 10, 70);
        btnOpenWnd.set_parent(this);
        btnOpenWnd.create(L"Open Window", 280, 30, 10, 100);

        cSystem.onChanged([this](EventData&) {
            if (!get_global_option(Option_EnableGlobalHotkey)) {
                PostQuitMessage(0x1231); // 重新运行消息循环
            }
        });

        btnOpenWnd.onClick([this](EventData&) {
            int mode = 0;
            if (cWindow.checked()) mode = 1;
            if (cThread.checked()) mode = 2;
            if (cSystem.checked()) mode = 3;
            if (mode == 0) {
                MessageBoxW(hwnd, L"Please select a hot key type.", L"Hot Key Type", MB_ICONERROR);
                return;
            }
            if (cchNextKey > 'Z') {
                MessageBoxW(hwnd, L"Hot key limit exceeded.", L"Hot Key Limit", MB_ICONERROR);
                return;
            }

            if ((mode == 3 && (cThread.checked() || cWindow.checked())) || (mode == 2 && cWindow.checked())) {
                MessageBoxW(hwnd, L"Conflict hot key type.", L"Hot Key Type", MB_ICONERROR);
                return;
            }

            HotKeyDemoWindow *ptr = new HotKeyDemoWindow(mode, cchNextKey++);
            ptr->create();
            ptr->center();
            ptr->show();
            windows.push_back(ptr);
        });
    }
    void onClose(EventData& ev) {
        if (!windows.empty()) {
            bool hasValid = false;
            for (auto& i : windows) {
                if (i && i->IsAlive()) {
                    hasValid = true;
                    break;
                }
            }
            if (!hasValid) {
                // free the memory
                for (auto& i : windows) {
                    delete i;
                }
                windows.clear();
            } else {
                ev.preventDefault();
                MessageBoxW(hwnd, L"You must close all child windows before you can quit the application.", L"Close Windows", MB_ICONERROR);
            }
        }
    }
    virtual void setup_event_handlers() override {
        WINDOW_add_handler(WM_CLOSE, onClose);
    }
};

// Using console so that we can see the debug output
int main(int argc, char* argv[]) {
    try {
        // Enabling debug mode
        Window::set_global_option(Window::GlobalOptions::Option_DebugMode, true);
        // Running your app
        HotKeyDemoMainApplication app;
        app.create();
        app.set_main_window();
        app.center();
        app.show();
        SetConsoleCtrlHandler([](DWORD dwCtrlType) {
            cout << "No way to Ctrl+C!" << endl;
            return TRUE;
        }, TRUE);
        Window::set_global_option(Window::GlobalOptions::Option_EnableHotkey, true);
        int code = app.run();
        if (code == 0x1231) {
            cout << "Re-run message loop to enable global-scope hot key..." << endl;
            Window::set_global_option(Window::GlobalOptions::Option_EnableGlobalHotkey, true);
            // 重新运行消息循环
            code = app.run();
        }
        return code;
    }
    catch (exception& exc) {
        int i = 0;
        TaskDialog(NULL, NULL, w32oop::util::str::converts::str_wstr(typeid(exc).name()).c_str(),
            L"Unexpected Exception was propagated to top-level stack",
            w32oop::util::str::converts::str_wstr(exc.what()).c_str(),
            TDCBF_CANCEL_BUTTON, TD_ERROR_ICON, &i);
        return GetLastError();
    }
}