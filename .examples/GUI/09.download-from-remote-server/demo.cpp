// w32oop Framework Example
//
// : 09.download-from-remote-server/demo.cpp
// This example demonstrates how to download a file from a remote server using w32oop Framework.

#include <w32use.hpp>
#include <iostream>
#include <sstream>

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
	ConstraintLayout layout;
	HMODULE winhttp;
	void onCreated() override {
		winhttp = LoadLibraryW(L"winhttp.dll");
		btn = Button(*this, L"Click me to download", 360, 30, 0, 0);
		btn.create();
		layout.define(btn)
			.constraintStart_toStartOf(nullptr).constraintEnd_toEndOf(nullptr)
			.constraintTop_toTopOf(nullptr)
			.width(ConstraintLayout::MATCH_CONSTRAINT).height(30)
			.marginStart(10).marginEnd(10).marginTop(10);
		btn.onClick([this] (EventData&) {
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
						textBox.text(L"Failed: " + ErrorChecker(GetLastError(), winhttp).message() + L"\r\n" + w32oop::util::str::converts::str_wstr(e.what()));
					}
				}, value.value()).detach();
				textBox.text(L"Download in progress...");
				addEventListener(WM_CLOSE, [this] (EventData& event) {
					event.preventDefault();
				});
			}
		});

		file = Edit(*this, L"", 360, 25, 0, 0);
		file.create();
		layout.define(file)
			.constraintStart_toStartOf(nullptr).constraintEnd_toEndOf(nullptr)
			.constraintTop_toTopOf(nullptr)
			.width(ConstraintLayout::MATCH_CONSTRAINT).height(25)
			.marginStart(10).marginEnd(10).marginTop(50);

		textBox = Edit(*this, L"", 360, 165, 0, 0, Edit::STYLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_WANTRETURN);
		textBox.create();
		layout.define(textBox)
			.constraintStart_toStartOf(nullptr).constraintEnd_toEndOf(nullptr)
			.constraintTop_toTopOf(nullptr).constraintBottom_toBottomOf(nullptr)
			.width(ConstraintLayout::MATCH_CONSTRAINT).height(ConstraintLayout::MATCH_CONSTRAINT)
			.marginStart(10).marginEnd(10).marginTop(85).marginBottom(15);
	}
protected:
    virtual void setup_event_handlers() override {
		setLayoutManager(&layout);
        WINDOW_add_handler(WM_USER + WM_CLOSE, [this](EventData&) {
            removeEventListener(WM_CLOSE);
        });
	}
};

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR pCmdLine,
	_In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pCmdLine);
	
	InternetAccessDemoWindow window;
    window.create();
    window.set_main_window();
    window.center();
    window.show(nCmdShow);
    return window.run();
}