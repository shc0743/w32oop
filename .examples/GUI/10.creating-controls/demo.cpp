// compile command: cl /EHsc demo.cpp /std:c++20
#define UNICODE 1
#define _UNICODE 1
#include <w32use.hpp>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(linker, "/entry:mainCRTStartup /subsystem:windows")

using namespace std;

class YourWindowClass : public Window {
public:
	YourWindowClass() : Window(L"Your Window Class", 400, 300, 0, 0, WS_OVERLAPPEDWINDOW) {}
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

int main() { 
	YourWindowClass window;
    window.create();
    window.set_main_window();
    window.center();
    window.show();
    return window.run();
}