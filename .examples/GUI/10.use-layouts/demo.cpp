// This demo is outdated! It won't work and do not use this anymore

// w32oop Framework Example
//
// : 10.use-layouts/demo.cpp
// This example demonstrates how to use layout managers.

#include <w32use.hpp>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;

class LayoutsDemoWindow : public Window {
public:
	LayoutsDemoWindow() : Window(L"Layout Managers Demo", 760, 520, 0, 0, WS_OVERLAPPEDWINDOW) {}

private:
	LinearLayout layout1;
	ConstraintLayout layout;

	Static text1;
	Button button2, button3, button4;

	void onCreated() override {
		layout1.orientation(LinearLayout::Orientation::Vertical);
		layout1.define(layout).width(300).margin(10).gravityCenter();

		text1.set_parent(hwnd);
		text1.create(L"Centered Text", 300, 1, 0, 0, text1.STYLE | SS_CENTER | SS_CENTERIMAGE);
		layout.define(text1)
			.constraintStart_toStartOf(nullptr).constraintTop_toTopOf(nullptr).constraintEnd_toEndOf(nullptr) // You can use nullptr to specify @parent
			.width(layout.WRAP_CONTENT).height(30);

		button2.set_parent(hwnd);
		button2.create(L"Button 2", 300, 1);
		layout.define(button2)
			.constraintStart_toStartOf(nullptr).constraintTop_toBottomOf(text1).constraintEnd_toEndOf(nullptr) // you can also specify it manually
			.width(layout.WRAP_CONTENT).height(30).marginTop(10);

		button3.set_parent(hwnd);
		button3.create(L"Button 3", 300, 1);
		button4.set_parent(hwnd);
		button4.create(L"Button 4", 300, 1);

		layout.define(button3)
			.constraintStart_toStartOf(nullptr).constraintTop_toBottomOf(button2).constraintEnd_toStartOf(button4)
			.width(layout.MATCH_CONSTRAINT).height(30).marginTop(10).marginRight(10);
		layout.define(button4)
			.constraintTop_toBottomOf(button2).constraintEnd_toEndOf(nullptr)
			.width(100).height(30).marginTop(10);

		// The layout is automatic after onCreated, so there is no need to call it again
	}

protected:
	virtual void setup_event_handlers() override {
		setLayoutManager(&layout1); // do not forget to set layout manager!
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

	LayoutsDemoWindow window;
	window.create();
	window.set_main_window();
	window.center();
	window.show(nCmdShow);
	return window.run();
}
