// w32oop Framework Example
//
// : 09.download-from-remote-server/demo.cpp
// This example demonstrates how to download a file from a remote server using w32oop Framework.

#include <w32use.hpp>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;

class InternetAccessDemoWindow : public Window {
public:
	InternetAccessDemoWindow() : Window(L"Access Internet Demo", 400, 300, 0, 0, WS_OVERLAPPEDWINDOW) {}

private:
	Button btn;
	Edit file;
	Edit textBox;
	void onCreated() override {
		btn = Button(*this, L"Click me to download", 360, 30, 10, 10);
		btn.create();
		btn.onClick([this] (EventData& event) {
			InputDialog idd; idd.create(); idd.center(hwnd);
			idd.setButtonsText(L"Download", L"Cancel");
			auto value = idd.getInput<wstring>(L"Input URL, Please.");
			if (value.has_value()) {
				thread([this](wstring url) {
					w32oop::util::RAIIHelper R([this] {
						post(WM_USER + WM_CLOSE);
					});
					try {
						HttpRequest req(url);
						req.file_buffer_file_name(file.text());
						HttpResponse resp = fetch(req);
						if (resp.isMemoryBody())
							textBox.text(resp.text());
						else textBox.text(L"OK: File is large. Saved to " + file.text());
					}
					catch (exception& e) {
						textBox.text(L"Failed: " + ErrorChecker().message() + L"\n" + w32oop::util::str::converts::str_wstr(e.what()));
					}
				}, value.value()).detach();
				textBox.text(L"Download in progress...");
				addEventListener(WM_CLOSE, [this] (EventData& event) {
					event.preventDefault();
				});
			}
		});

		file = Edit(*this, L"", 360, 25, 10, 50);
		file.create();

		textBox = Edit(*this, L"", 360, 165, 10, 85, Edit::STYLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_WANTRETURN);
		textBox.create();
	}
	void doLayout(EventData& ev) {
		RECT rc{}; GetClientRect(hwnd, &rc);
		auto w = rc.right - rc.left, h = rc.bottom - rc.top;
		// 调整控件大小
		btn.resize(10, 10, w - 20, 30);
		file.resize(10, 50, w - 20, 25);
		textBox.resize(10, 85, w - 20, h - 100);
	}
protected:
    virtual void setup_event_handlers() override {
        WINDOW_add_handler(WM_USER + WM_CLOSE, [this](EventData& event) {
            removeEventListener(WM_CLOSE);
        });
		WINDOW_add_handler(WM_SIZE, doLayout);
		WINDOW_add_handler(WM_SIZING, doLayout);
	}
};

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR pCmdLine,
	_In_ int nCmdShow)
{
	InternetAccessDemoWindow window;
    window.create();
    window.set_main_window();
    window.center();
    window.show();
    return window.run();
}