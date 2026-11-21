// w32oop Framework Example
//
// : 10.use-layouts/demo.cpp
// This example demonstrates the flexbox layout engine.

#include <w32use.hpp>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;

class LayoutsDemoWindow : public Window {
public:
	LayoutsDemoWindow() : Window(L"Flexbox Layout Demo", 760, 520, 0, 0, WS_OVERLAPPEDWINDOW) {}

private:
	// Layout objects live as long as the window. root refers to the nested
	// layouts below, so they must outlive root's last layout pass (they do:
	// members are destroyed after the base Window is gone).
	FlexLayout root{ Direction::Column };
	FlexLayout header{ Direction::Row };
	FlexLayout toolbar{ Direction::Row };
	FlexLayout tags{ Direction::Row };

	Static title;
	Static hint;
	Edit editor;
	Button btnNew, btnOpen, btnSave, btnApply;
	Static tagA, tagB, tagC, tagD, tagE;
	Static status;

	void onCreated() override {
		// 1. Create the controls. The parent is always *this.
		title = Static(*this, L"Flexbox layout", 160, 24);
		title.create();

		hint = Static(*this, L"Resize the window", 150, 24);
		hint.create();

		btnNew = Button(*this, L"New", 96, 30);
		btnNew.create();
		btnNew.onClick([this](EventData&) { status.text(L"New clicked."); });

		btnOpen = Button(*this, L"Open", 96, 30);
		btnOpen.create();
		btnOpen.onClick([this](EventData&) { status.text(L"Open clicked."); });

		btnSave = Button(*this, L"Save", 96, 30);
		btnSave.create();
		btnSave.onClick([this](EventData&) { status.text(L"Save clicked."); });

		btnApply = Button(*this, L"Apply", 96, 30);
		btnApply.create();
		btnApply.onClick([this](EventData&) {
			status.text(L"Editor content: " + editor.text());
		});

		editor = Edit(*this, L"Type something here...", 100, 100, 0, 0,
			Edit::STYLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL);
		editor.create();

		tagA = Static(*this, L"layout", 86, 24, 0, 0, Static::STYLE | SS_CENTER);
		tagA.create();
		tagB = Static(*this, L"flexbox", 86, 24, 0, 0, Static::STYLE | SS_CENTER);
		tagB.create();
		tagC = Static(*this, L"grow", 86, 24, 0, 0, Static::STYLE | SS_CENTER);
		tagC.create();
		tagD = Static(*this, L"shrink", 86, 24, 0, 0, Static::STYLE | SS_CENTER);
		tagD.create();
		tagE = Static(*this, L"wrap", 86, 24, 0, 0, Static::STYLE | SS_CENTER);
		tagE.create();

		status = Static(*this, L"Ready.", 100, 22);
		status.create();

		// 2. Describe the layout. No manual relayout calls are needed: adding
		//    items and changing styles schedules one reflow automatically.
		root.padding(12).gap(10);

		// header: title on the left, hint on the right
		root.add(header).height(28);
		header.justifyContent(Justify::SpaceBetween).alignItems(Align::Center);
		header.add(title).width(160);
		header.add(hint).width(150);

		// toolbar: equal-width buttons, gap between them, stretched to 32px high
		root.add(toolbar).height(32);
		toolbar.gap(8);
		toolbar.add(btnNew).width(96);
		toolbar.add(btnOpen).width(96);
		toolbar.add(btnSave).width(96);
		toolbar.add(btnApply).width(96);

		// tags: wraps into a second line when the window gets narrow
		root.add(tags).height(56);
		tags.gap(6).wrap(true);
		tags.alignContent(AlignContent::Start);
		tags.add(tagA).width(86).height(24);
		tags.add(tagB).width(86).height(24);
		tags.add(tagC).width(86).height(24);
		tags.add(tagD).width(86).height(24);
		tags.add(tagE).width(86).height(24);

		// editor: flex: 1 1 0 — it absorbs all remaining vertical space
		root.add(editor).grow(1).basis(0).minHeight(0).margin(LayoutInsets::symmetric(0, 2));

		// status bar: fixed height
		root.add(status).height(22);
	}

protected:
	virtual void setup_event_handlers() override {
		// Bind the root layout. From now on the window reflows on WM_SIZE and
		// whenever the layout tree changes.
		set_layout(root);
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
