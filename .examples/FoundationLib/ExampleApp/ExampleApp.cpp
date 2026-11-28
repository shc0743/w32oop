#include "../../../w32use.hpp"
#include "examples_registry.hpp"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "comctl32.lib") 

using namespace std;

#pragma optimize("", off)

class MainWindowClass : public Window {
public:
	MainWindowClass() : Window(L"w32oop Foundation Libraries Example", 640, 480, 0, 0, WS_OVERLAPPEDWINDOW) {}
private:
	ListBox ChooseExample;
	shared_ptr<Window> lpExample;
	void onCreated() override {
		auto tips = make_shared<StaticEx>(hwnd, L"Please select one of the examples from the list left.", 
			1, 1, 0, 0, StaticEx::STYLE | SS_CENTER | SS_CENTERIMAGE);
		tips->create();
		tips->backgroundColor(RgbColor("#fff"));
		lpExample = tips;
		
		ChooseExample.set_parent(this);
		ChooseExample.create(L"", 1, 1);
		ChooseExample.onChange([this](EventData&) {
			lpExample = nullptr;
			auto sel = ChooseExample.getCurrentSelection();
			if (sel < 0 || sel >= Examples.size()) {
				return;
			}
			lpExample = Examples.at(sel).second(hwnd);
			if (lpExample) {
				lpExample->create();
				lpExample->add_style(WS_BORDER);
				lpExample->add_style_ex(WS_EX_CONTROLPARENT);
				lpExample->show();
			}
			doLayout();
		});
		for (auto& i : Examples) ChooseExample.addItem(i.first);
	}
protected:
	void doLayout() {
		RECT rc = client_rect();
		if (ChooseExample) ChooseExample.resize(10, 10, 100, rc.bottom - rc.top - 20);
		if (lpExample) lpExample->resize(120, 10, rc.right - rc.left - 100 - 20 - 10, rc.bottom - rc.top - 20);
	}
	virtual void setup_event_handlers() override {
		WINDOW_add_handler(WM_SIZE, [this](EventData&) { doLayout(); });
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
	INITCOMMONCONTROLSEX icce{};
	icce.dwSize = sizeof(icce);
	icce.dwICC = ICC_ALL_CLASSES;
	InitCommonControlsEx(&icce);

	Window::set_global_option(Window::Option_DebugMode, 1);
	MainWindowClass window;
	window.create();
	window.set_main_window();
	window.center();
	window.show(nCmdShow);
	return window.run();
}
