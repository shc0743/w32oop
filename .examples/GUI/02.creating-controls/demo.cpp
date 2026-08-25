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
	void onCreated() override {
		// lifecycle hooks
		text = StaticEx(*this, L"...", 470, 60, 10, 90);
		text.color(RgbColor("#abcdef"));
		text.backgroundColor(RgbColor(0x123456));
		text.create();

		btn = Button(*this, L"Click me!", 100, 30, 10, 10);
		btn.create();
		btn.onClick([this] (EventData& event) {
			text.text(L"Hello, World!");
		});

		textBox = Edit(*this, L"Hello, World!", 200, 30, 10, 50);
		textBox.create();
		textBox.onChange([this] (EventData& event) {
			text.text(L"Text area edited");
		});

		textBoxColorSetter = Edit(*this, L"#abcdef", 120, 30, 10, 160);
		textBoxColorSetter.create();
		textBoxColorSetter.onChange([this] (EventData& event) {
			try {
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
        
    }
};

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR pCmdLine,
	_In_ int nCmdShow)
{
	YourWindowClass window;
    window.create();
    window.set_main_window();
    window.center();
    window.show();
    return window.run();
}