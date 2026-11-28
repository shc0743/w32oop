#pragma once
#include "../../../w32use.hpp"

// Edit example: one edit box and a few buttons that call Edit's methods.
class EditExample : public ChildControlWindow {
public:
	EditExample(HWND parent) : ChildControlWindow(parent, L"Edit example", 1, 1) {}

protected:
	ConstraintLayout layout;
	LinearLayout coreLayout;
	Edit edit;
	Static tips;
	Button btnUndo, btnRedo, btnMaxLength, btnPassword, btnLineCount, btnLine;
	CheckBox chkReadonly;

	void setup_event_handlers() override {
		setLayoutManager(&coreLayout);
	}

	void onCreated() override {
		// ES_MULTILINE makes line_count() and get_line() useful
		edit.set_parent(this);
		edit.create(L"Hello, w32oop!", 1, 1,
			0, 0, Edit::STYLE | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL);
		layout.define(edit).width(400).height(120) 
			.centerHorizontally().constraintTop_toTopOf(LayoutRef{}).marginTop(20);

		// buttons: set_parent, create(text, width, height), then place with the layout
		btnUndo.set_parent(this);
		btnUndo.create(L"Undo", 1, 1);
		layout.define(btnUndo).width(70).height(28)
			.constraintStart_toStartOf(edit).constraintTop_toBottomOf(edit).marginTop(16);

		btnRedo.set_parent(this);
		btnRedo.create(L"Redo", 1, 1);
		layout.define(btnRedo).width(70).height(28)
			.constraintStart_toEndOf(btnUndo).marginStart(8).constraintTop_toTopOf(btnUndo);

		btnMaxLength.set_parent(this);
		btnMaxLength.create(L"Set Max Length", 1, 1);
		layout.define(btnMaxLength).width(110).height(28)
			.constraintStart_toEndOf(btnRedo).marginStart(8).constraintTop_toTopOf(btnUndo);

		btnPassword.set_parent(this);
		btnPassword.create(L"Set Password Char", 1, 1);
		layout.define(btnPassword).width(135).height(28)
			.constraintStart_toEndOf(btnMaxLength).marginStart(8).constraintTop_toTopOf(btnUndo);

		btnLineCount.set_parent(this);
		btnLineCount.create(L"Get Line Count", 1, 1);
		layout.define(btnLineCount).width(120).height(28)
			.constraintStart_toStartOf(edit).constraintTop_toBottomOf(btnUndo).marginTop(8);

		btnLine.set_parent(this);
		btnLine.create(L"Get Line", 1, 1);
		layout.define(btnLine).width(90).height(28)
			.constraintStart_toEndOf(btnLineCount).marginStart(8).constraintTop_toTopOf(btnLineCount);

		chkReadonly.set_parent(this);
		chkReadonly.create(L"Readonly", 1, 1);
		layout.define(chkReadonly).width(110).height(28)
			.constraintStart_toEndOf(btnLine).marginStart(8).constraintTop_toTopOf(btnLineCount);

		// the label shows the result of Get Line Count / Get Line
		tips.set_parent(this);
		tips.create(L"", 1, 1);
		layout.define(tips).width(400).height(24)
			.constraintStart_toStartOf(edit).constraintTop_toBottomOf(btnLineCount).marginTop(8);

		btnUndo.onClick([this](EventData&) { edit.undo(); });
		btnRedo.onClick([this](EventData&) { edit.redo(); });

		btnMaxLength.onClick([this](EventData&) {
			InputDialog idd(L"Input max length", 500, 145, hwnd);
			edit.max_length(idd.getInput<int>(L"Please input new max length (0 = no limit)").value_or(0));
		});

		// toggle between a password char and plain text
		btnPassword.onClick([this](EventData&) {
			edit.password_char(edit.password_char() ? L'\0' : L'*');
			edit.update();
		});

		btnLineCount.onClick([this](EventData&) {
			tips.text(L"Line count: " + std::to_wstring(edit.line_count()));
		});

		btnLine.onClick([this](EventData&) {
			tips.text(L"Line 0: " + edit.get_line(0));
		});

		chkReadonly.onChanged([this](EventData&) {
			edit.readonly(chkReadonly.checked());
		});

		coreLayout.define(layout).gravityCenter();
	}
};
