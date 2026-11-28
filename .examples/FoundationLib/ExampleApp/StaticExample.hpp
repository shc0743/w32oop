#pragma once
#include "../../../w32use.hpp"

// Static example: plain text labels with the three alignments.
class StaticExample : public ChildControlWindow {
public:
	StaticExample(HWND parent) : ChildControlWindow(parent, L"Static example", 1, 1) {}

protected:
	LinearLayout coreLayout;
	ConstraintLayout layout;
	Static lblLeft, lblCenter, lblRight;
	Button btnChange;

	void setup_event_handlers() override {
		setLayoutManager(&coreLayout);
	}

	void onCreated() override {
		// Static is the system text control, the style decides the alignment
		lblLeft.set_parent(this);
		lblLeft.create(L"Left aligned", 1, 1, 0, 0, Static::STYLE | SS_LEFT);
		layout.define(lblLeft).width(260).height(24)
			.centerHorizontally().constraintTop_toTopOf(LayoutRef{});

		lblCenter.set_parent(this);
		lblCenter.create(L"Center aligned", 1, 1, 0, 0, Static::STYLE | SS_CENTER);
		layout.define(lblCenter).width(260).height(24)
			.centerHorizontally().constraintTop_toBottomOf(lblLeft).marginTop(8);

		lblRight.set_parent(this);
		lblRight.create(L"Right aligned", 1, 1, 0, 0, Static::STYLE | SS_RIGHT);
		layout.define(lblRight).width(260).height(24)
			.centerHorizontally().constraintTop_toBottomOf(lblCenter).marginTop(8);

		btnChange.set_parent(this);
		btnChange.create(L"Change text", 1, 1);
		layout.define(btnChange).width(140).height(28)
			.centerHorizontally().constraintTop_toBottomOf(lblRight).marginTop(16);

		// the text of a Static can be changed at any time
		btnChange.onClick([this](EventData&) {
			lblRight.text(L"Text changed by the button");
		});

		// put the whole block in the middle of the example area
		coreLayout.define(layout).gravityCenter();
	}
};
