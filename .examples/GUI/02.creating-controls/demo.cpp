// w32oop Framework Example
//
// : 02.creating-controls/demo.cpp
// This example demonstrates how to create and use various controls in a GUI application using the w32oop framework.

#include <w32use.hpp>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;

class YourWindowClass : public Window {
public:
	YourWindowClass() : Window(L"Your Window Title", 500, 300, 0, 0, WS_OVERLAPPEDWINDOW) {}
private:
	StaticEx text;
	Button btn;
	Edit textBox, textBoxColorSetter;
	ConstraintLayout layout; // You will learn this soon! This is much more convinent than manual resizing (but you can still do it manually)
	void onCreated() override {
		// lifecycle hooks
		text = StaticEx(hwnd, L"...", 470, 60, 0, 0); // `hwnd` is the internal HWND that is available in the whole class, you can use `hwnd` directly at any time within the class
		text.color(RgbColor("#abcdef")); // there is a convenient utility for you to convert any hex colors to COLORREF! It is even constexpr so no runtime cost is here
		text.backgroundColor(RgbColor(0x123456)); // hex like this is also supported
		text.create();
		layout.define(text)
			.constraintStart_toStartOf(nullptr).constraintTop_toTopOf(nullptr)
			.width(470).height(60).marginStart(10).marginTop(90);

		btn = Button(hwnd, L"Click me!", 100, 30, 0, 0);
		btn.create();
		layout.define(btn)
			.constraintStart_toStartOf(nullptr).constraintTop_toTopOf(nullptr)
			.width(100).height(30).marginStart(10).marginTop(10);
		btn.onClick([this] (EventData& event) {
		    UNREFERENCED_PARAMETER(event);
			text.text(L"Hello, World!");
		});

		textBox = Edit(*this, L"Hello, World!", 200, 30, 0, 0); // you can also use the implicit operator, but it is much more ugly than directly use `hwnd`
		textBox.create();
		layout.define(textBox)
			.constraintStart_toStartOf(nullptr).constraintTop_toTopOf(nullptr)
			.width(200).height(30).marginStart(10).marginTop(50);
		textBox.onChange([this] (EventData& event) {
		    UNREFERENCED_PARAMETER(event);
			text.text(L"Text area edited");
		});

		textBoxColorSetter = Edit(*this, L"#abcdef", 120, 30, 0, 0);
		textBoxColorSetter.create();
		layout.define(textBoxColorSetter)
			.constraintStart_toStartOf(nullptr).constraintTop_toTopOf(nullptr)
			.width(120).height(30).marginStart(10).marginTop(160);
		textBoxColorSetter.onChange([this] (EventData& event) {
		    UNREFERENCED_PARAMETER(event);
			try {
				// If you want to parse the COLORREF at runtime, just use ParseRgbColorRt!
				text.color(ParseRgbColorRt(w32oop::util::str::encodings::utf16_utf8(textBoxColorSetter.text())));
				text.text(L"Color updated!!");
			}
			catch (exception& e) {
				text.text(w32oop::util::str::encodings::utf8_utf16(string(typeid(e).name()) + "\r\n" + e.what()));
			}
		});
	}
	void onDestroy() override {
		// lifecycle hooks
		// you can do your cleanup here
	}
protected:
    virtual void setup_event_handlers() override {
		setLayoutManager(&layout);
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
	UNREFERENCED_PARAMETER(nCmdShow);
	YourWindowClass window;
    window.create();
    window.set_main_window();
    window.center();
    window.show();
    return window.run();
}