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
	YourWindowClass() : Window(L"Your Window Title", 400, 300, 0, 0, WS_OVERLAPPEDWINDOW) {}
private:
	Static text;
	Button btn;
	Edit textBox;
	void onCreated() override {
		// lifecycle hooks
		text.set_parent(this); // Must call
		text.create(L"...", 250, 30, 10, 90);

		btn.set_parent(this); // Must call
		btn.create(L"Click me!", 100, 30, 10, 10);
		btn.onClick([this] (EventData& event) {
			text.text(L"Hello, World!");
		});

		textBox.set_parent(this); // Must call
		textBox.create(L"Hello, World!", 200, 30, 10, 50);
		textBox.onChange([this] (EventData& event) {
			text.text(L"Text area edited");
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