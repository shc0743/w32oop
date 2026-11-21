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
	Static title;
	Static status;
	Edit editor;
	Button btnNew, btnOpen, btnSave, btnApply;

	LinearLayout rootLayout;
	LinearLayout bodyLayout;
	LinearLayout leftColumn;
	ConstraintLayout rightPane;

	void relayout() {
		if (rootLayout.owner_window()) rootLayout.doLayout();
	}

	void onCreated() override {
		// 1. Create the controls first.
		title = Static(*this, L"LinearLayout + ConstraintLayout (nested)", 100, 36);
		title.create();

		status = Static(*this, L"Resize the window to reflow the layout.", 100, 24);
		status.create();

		btnNew = Button(*this, L"New", 100, 30);
		btnNew.create();
		btnNew.onClick([this](EventData&) { status.text(L"New clicked."); });

		btnOpen = Button(*this, L"Open", 100, 30);
		btnOpen.create();
		btnOpen.onClick([this](EventData&) { status.text(L"Open clicked."); });

		btnSave = Button(*this, L"Save", 100, 30);
		btnSave.create();
		btnSave.onClick([this](EventData&) { status.text(L"Save clicked."); });

		btnApply = Button(*this, L"Apply", 100, 30);
		btnApply.create();
		btnApply.onClick([this](EventData&) {
			status.text(L"Editor content: " + editor.text());
		});

		editor = Edit(*this, L"Type something here...", 100, 100, 0, 0,
			Edit::STYLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL);
		editor.create();

		// 2. Attach the owner now that the window exists.
		rootLayout.set_owner(this);
		bodyLayout.set_owner(this);
		bodyLayout.orientation(LinearLayout::Orientation::Horizontal);
		leftColumn.set_owner(this);
		rightPane.set_owner(this);

		// 3. Describe the layout: root = title / body / status bar.
		rootLayout.define(title)
			.matchParent()
			.height(36)
			.margin(12);

		rootLayout.define(bodyLayout)
			.matchParent()
			.weight(1.0f)
			.margin(12);

		rootLayout.define(status)
			.matchParent()
			.height(24)
			.margin(12);

		// 4. Body = button column on the left + constraint pane on the right; weight splits the width.
		bodyLayout.define(leftColumn)
			.matchParent()
			.weight(1.0f)
			.marginEnd(8);

		bodyLayout.define(rightPane)
			.matchParent()
			.weight(2.0f)
			.marginStart(8);

		leftColumn.define(btnNew).matchParent().height(32).marginBottom(6);
		leftColumn.define(btnOpen).matchParent().height(32).marginBottom(6);
		leftColumn.define(btnSave).matchParent().height(32).marginBottom(6);
		leftColumn.define(btnApply).matchParent().height(32);

		// 5. Right ConstraintLayout: editor spans the width, Apply is pinned to the bottom-right.
		rightPane.define(editor)
			.width(ConstraintLayout::MATCH_CONSTRAINT)
			.height(140)
			.constraintTop_toTopOf(nullptr)
			.constraintStart_toStartOf(nullptr)
			.constraintEnd_toEndOf(nullptr)
			.margin(8);

		rightPane.define(btnApply)
			.size(120, 32)
			.constraintEnd_toEndOf(nullptr)
			.constraintBottom_toBottomOf(nullptr)
			.margin(8);

		// 6. First layout pass.
		relayout();
	}

protected:
	virtual void setup_event_handlers() override {
		setLayoutManager(&rootLayout);
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
