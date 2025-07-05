// compile command: cl /EHsc demo.cpp /std:c++20
#define UNICODE 1
#define _UNICODE 1
#include <w32use.hpp>
#include <fstream>
// To simplify the code, we included a CPP
// however, never do it in a real project!
#include "utfutil.cpp"
#include <CommCtrl.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(linker, "/entry:wmainCRTStartup /subsystem:windows")

using namespace std;

namespace MyDemo {

    ULONGLONG MyGetFileSizeW(wstring filename)
    {
        GET_FILEEX_INFO_LEVELS fInfoLevelId = GetFileExInfoStandard;
        WIN32_FILE_ATTRIBUTE_DATA FileInformation = {0};
        // 通过winapi获取文件属性
        BOOL bGet = GetFileAttributesExW(filename.c_str(), fInfoLevelId, &FileInformation);
        if (bGet)
        {
            _fsize_t nFileSizeLow = FileInformation.nFileSizeLow;
            _fsize_t nFileSizeHigh = FileInformation.nFileSizeHigh;
            ULARGE_INTEGER szFile{};
            szFile.HighPart = nFileSizeHigh; // 超大文件才需要
            szFile.LowPart = nFileSizeLow;
            return szFile.QuadPart;
        }
        return 0;
    }

    class SimpleNotePadApplication : public Window {
    private:
        Button btnOpen;
        Button btnSave, btnSaveAs;
        Edit lblFilePath;
        Edit txtEditor;
        std::wstring currentFilePath; // 当前文件路径
        HFONT editorFont = NULL;

        bool unsaved = false;

    public:
        SimpleNotePadApplication(const wstring &title, int width, int height)
            : Window(title, width, height, 0, 0, WS_OVERLAPPEDWINDOW) {}

    protected:
        void onCreated() override {
            // 创建 [打开文件] 按钮
            btnOpen.set_parent(*this);
            btnOpen.create(L"打开文件", 80, 30, 10, 10);
            btnOpen.onClick([this](EventData &) { openFile(); });

            // 创建 [保存文件] 按钮
            btnSave.set_parent(*this);
            btnSave.create(L"保存文件", 80, 30, 100, 10);
            btnSave.onClick([this](EventData &) { saveFile(); });

            // 创建 [另存为] 按钮
            btnSaveAs.set_parent(*this);
            btnSaveAs.create(L"另存为", 80, 30, 100, 10);
            btnSaveAs.onClick([this](EventData &) { saveFile(true); });

            // 创建文件路径显示框（只读）
            lblFilePath.set_parent(*this);
            lblFilePath.create(L"当前文件：无", 300, 30, 200, 10);
            lblFilePath.readonly(true);

            // 创建字体
            editorFont = CreateFontW(-18, -9, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE,
                L"NSimsun");

            // 创建多行编辑框
            txtEditor.set_parent(*this);
            txtEditor.create(L"", 620, 400, 10, 50, 
                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_HSCROLL | WS_VSCROLL);
            txtEditor.onChange([&](EventData &) { 
                if (!unsaved) {
                    btnSave.text(L"请保存!");
                }
                unsaved = true; 
            });
            txtEditor.font(editorFont);

            register_hot_key(true, false, false, 'W', [this](HotKeyProcData& event){
                event.preventDefault();
                // 线程模式钩子需要一些额外处理
                int repeat_count = event.lParam & 0x0000FFFF;
                if (repeat_count > 1) return;
                int released_key = (event.lParam >> 31) & 1;
                if (released_key) return;

                close();
            }, HotKeyOptions::Windowed);
            register_hot_key(true, false, false, 'O', [this](HotKeyProcData& event){
                event.preventDefault();
                // 线程模式钩子需要一些额外处理
                int repeat_count = event.lParam & 0x0000FFFF;
                if (repeat_count > 1) return;
                int released_key = (event.lParam >> 31) & 1;
                if (released_key) return;

                thread([this](){ openFile(); }).detach();
            }, HotKeyOptions::Windowed);
            register_hot_key(true, false, false, 'S', [this](HotKeyProcData& event){
                event.preventDefault();
                // 线程模式钩子需要一些额外处理
                int repeat_count = event.lParam & 0x0000FFFF;
                if (repeat_count > 1) return;
                int released_key = (event.lParam >> 31) & 1;
                if (released_key) return;
                thread([this](){ saveFile(); }).detach();
            }, HotKeyOptions::Windowed);
            register_hot_key(true, false, true, 'S', [this](HotKeyProcData& event){
                event.preventDefault();
                // 线程模式钩子需要一些额外处理
                int repeat_count = event.lParam & 0x0000FFFF;
                if (repeat_count > 1) return;
                int released_key = (event.lParam >> 31) & 1;
                if (released_key) return;
                thread([this](){ saveFile(true); }).detach();
            }, HotKeyOptions::Windowed); // Ctrl+Shift+S

            add_style_ex(WS_EX_ACCEPTFILES); // 允许接受文件
        }

    public:
        void loadFile(wstring filePath) {
            currentFilePath = filePath;
            lblFilePath.text(filePath);
            HANDLE hFile = CreateFileW(currentFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hFile == INVALID_HANDLE_VALUE) {
                MessageBoxW(hwnd, (L"对不起！我打不开这个文件！原因是：" + to_wstring(GetLastError())).c_str(), L"Sorry!", MB_ICONERROR);
                return;
            }
            char* buffer = new char[1024 * 1024 * 32 + 1]; // 32MB
            DWORD bytesRead = 0;
            if (ReadFile(hFile, buffer, 1024 * 1024 * 32, &bytesRead, nullptr)) {
                wstring u16 = ConvertUTF8ToUTF16(buffer);
                txtEditor.text(u16);
            } else {
                MessageBoxW(hwnd, (L"对不起！我读不出来这个文件！原因是：" + to_wstring(GetLastError())).c_str(), L"Sorry!", MB_ICONERROR);
            }
            CloseHandle(hFile);
            delete[] buffer;
        }
        // 打开文件
        void openFile() {
            wchar_t filePath[MAX_PATH] = {0};
            OPENFILENAME ofn = {0};
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = L"所有文件 (*.*)\0*.*\0";
            ofn.lpstrFile = filePath;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST;

            if (unsaved) {
                int result = MessageBoxW(hwnd, L"你有未保存的更改，是否放弃？", L"提示", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2); 
                if (result == IDNO) return;
                unsaved = false;
                btnSave.text(L"保存文件");
            }

            if (GetOpenFileName(&ofn)) {
                lblFilePath.text(filePath);

                if (MyGetFileSizeW(filePath) > 1024 * 1024 * 32) { // 32MB
                    MessageBoxW(hwnd, L"对不起！文件太大了，我打不开它！", L"Sorry!", MB_ICONERROR);
                    return;
                }

                loadFile(filePath);
            }
        }

        // 保存文件
        void saveFile(bool saveas = false) {
            // 如果没有打开过文件，弹出“另存为”对话框
            if (saveas || currentFilePath.empty()) {
                wchar_t filePath[MAX_PATH] = {0};
                OPENFILENAME ofn = {0};
                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFilter = L"所有文件 (*.*)\0*.*\0";
                ofn.lpstrFile = filePath;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_EXPLORER;

                if (!GetSaveFileName(&ofn))
                    return;
                currentFilePath = filePath;
                lblFilePath.text(currentFilePath);
            }

            // 写入文件
            HANDLE hFile = CreateFileW(currentFilePath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hFile == INVALID_HANDLE_VALUE) {
                MessageBoxW(hwnd, (L"对不起！我不能保存这个文件！原因是：" + to_wstring(GetLastError())).c_str(), L"Sorry!", MB_ICONERROR); 
                return;
            }
            string u8 = ConvertUTF16ToUTF8(txtEditor.text());
            DWORD bytesWritten = 0;
            if (!WriteFile(hFile, u8.c_str(), u8.length(), &bytesWritten, nullptr)) {
                MessageBoxW(hwnd, (L"对不起！我写不了这个文件！原因是：" + to_wstring(GetLastError())).c_str(), L"Sorry!", MB_ICONERROR);
            } else {
                unsaved = false;
                btnSave.text(L"保存文件");
            }
            CloseHandle(hFile);
        }

    private:
        void onSizeChange(EventData &e) {
            RECT rc{}; GetClientRect(hwnd, &rc);
            int btnHeight = 30, btnWidth = 80, btnMargin = 10;
            int lblHeight = 30, lblWidth = 300;
            int editorMarginTop = 50;
            int editorMarginSide = 10;
            // 按钮
            btnOpen.resize(btnWidth, btnHeight);
            btnOpen.move(btnMargin, btnMargin);

            btnSave.resize(btnWidth, btnHeight);
            btnSave.move(btnMargin + btnWidth + btnMargin, btnMargin);

            btnSaveAs.resize(btnWidth, btnHeight);
            btnSaveAs.move(btnMargin + 2 * (btnWidth + btnMargin), btnMargin);

            // 标签
            int lblLeft = btnMargin + 3 * (btnWidth + btnMargin);
            int lblWidthDynamic = rc.right - lblLeft - btnMargin;
            if (lblWidthDynamic < 0) lblWidthDynamic = 0;
            lblFilePath.resize(lblWidthDynamic, lblHeight);
            lblFilePath.move(lblLeft, btnMargin);

            // 编辑框
            int editorTop = editorMarginTop;
            int editorLeft = editorMarginSide;
            int editorWidth = rc.right - 2 * editorMarginSide;
            int editorHeight = rc.bottom - editorMarginTop - editorMarginSide;
            if (editorWidth < 0) editorWidth = 0;
            if (editorHeight < 0) editorHeight = 0;
            txtEditor.resize(editorWidth, editorHeight);
            txtEditor.move(editorLeft, editorTop);
        }

        void onWillClose(EventData &e) {
            if (!unsaved) return;
            e.preventDefault();
            int r = 0;
            TaskDialog(hwnd, NULL, L"未保存的更改!", L"你有未保存的更改，是否保存？", 
                L"如果不保存，更改将丢失！", TDCBF_YES_BUTTON | TDCBF_NO_BUTTON | TDCBF_CANCEL_BUTTON,
                TD_WARNING_ICON, &r);
            if (r == IDNO) {
                // 直接关闭
                unsaved = false;
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
            else if (r == IDYES) {
                // 保存
                saveFile();
                if (!unsaved) {
                    // 然后关闭
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                }
            }
        }
        void onWillShutdown(EventData &e) {
            if (!unsaved) return;
            e.returnValue(0);
        }
        void onDrop(EventData &e) {
            e.preventDefault();

            HDROP hDrop = (HDROP)e.wParam;
            UINT nFiles = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
            if (nFiles < 1) return;

            if (nFiles > 1) {
                MessageBoxW(hwnd, L"对不起！我一次只能打开一个文件！", L"Sorry!", MB_ICONERROR);
                return; 
            }
            auto filePath = std::make_unique<wchar_t[]>(32768);
            DragQueryFileW(hDrop, 0, filePath.get(), 32768); // 支持长路径
            loadFile(filePath.get());
        }

        virtual void setup_event_handlers() override {
            WINDOW_add_handler(WM_SIZING, onSizeChange);
            WINDOW_add_handler(WM_SIZE, onSizeChange);
            WINDOW_add_handler(WM_CLOSE, onWillClose);
            WINDOW_add_handler(WM_QUERYENDSESSION, onWillShutdown);
            WINDOW_add_handler(WM_ENDSESSION, onWillShutdown);
            WINDOW_add_handler(WM_DROPFILES, onDrop);
        }
    };

}

int wmain(int argc, wchar_t* argv[]) {
    // create the application
    MyDemo::SimpleNotePadApplication app(L"My Notepad", 640, 480);
    // create it
    app.create();
    // set the main window
    app.set_main_window();
    // center
    app.center();
    // show it
    app.show();
    thread([&]() {
        Sleep(500); // 简单的demo，忽略一些细节，假设500ms足够
        if (argc < 2) return;
        app.loadFile(argv[1]);
    }).detach();
    // message loop
    return Window::run();
}