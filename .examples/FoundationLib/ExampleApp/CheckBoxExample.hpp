#pragma once
#include "../../../w32use.hpp"

// CheckBox example: read the state and react to changes.
class CheckBoxExample : public ChildControlWindow {
public:
	CheckBoxExample(HWND parent) : ChildControlWindow(parent, L"CheckBox example", 1, 1) {}

protected:
	LinearLayout coreLayout;
	ConstraintLayout layout;
	CheckBox chk;
	Static lbl;
	Button btnUncheck;

	void setup_event_handlers() override {
		setLayoutManager(&coreLayout);
	}

	void onCreated() override {
		chk.set_parent(this);
		chk.create(L"Check me", 1, 1);
		layout.define(chk).width(180).height(28)
			.centerHorizontally().constraintTop_toTopOf(LayoutRef{});

		lbl.set_parent(this);
		lbl.create(L"Checked: no", 1, 1);
		layout.define(lbl).width(260).height(24)
			.centerHorizontally().constraintTop_toBottomOf(chk).marginTop(12);

		btnUncheck.set_parent(this);
		btnUncheck.create(L"Uncheck", 1, 1);
		layout.define(btnUncheck).width(120).height(28)
			.centerHorizontally().constraintTop_toBottomOf(lbl).marginTop(12);

		chk.onChanged([this](EventData&) {
			lbl.text(chk.checked() ? L"Checked: yes" : L"Checked: no");
		});

		// check() does not send BN_CLICKED, so the label is updated here too
		btnUncheck.onClick([this](EventData&) {
			chk.check(false);
			lbl.text(L"Checked: no");
		});

		coreLayout.define(layout).gravityCenter();
	}
};
