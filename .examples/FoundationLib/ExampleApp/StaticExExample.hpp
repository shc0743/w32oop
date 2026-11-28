#pragma once
#include "../../../w32use.hpp"

// StaticEx example: Static with custom text color and background color.
class StaticExExample : public ChildControlWindow {
public:
	StaticExExample(HWND parent) : ChildControlWindow(parent, L"StaticEx example", 1, 1) {}

protected:
	LinearLayout coreLayout;
	ConstraintLayout layout;
	StaticEx boxRed, boxBlue, boxGreen;
	Button btnChange;

	void setup_event_handlers() override {
		setLayoutManager(&coreLayout);
	}

	void onCreated() override {
		// StaticEx wraps a Static and adds color() / backgroundColor().
		// Set the colors before create() so the first paint already uses them.
		boxRed.color(RgbColor("#c00000"));
		boxRed.backgroundColor(RgbColor("#ffffff"));
		boxRed.set_parent(this);
		boxRed.create(L"Red text on white background", 1, 1,
			0, 0, StaticEx::STYLE | SS_CENTER | SS_CENTERIMAGE);
		layout.define(boxRed).width(320).height(40)
			.centerHorizontally().constraintTop_toTopOf(LayoutRef{});

		boxBlue.color(RgbColor("#0000c0"));
		boxBlue.backgroundColor(RgbColor("#ffffe0"));
		boxBlue.set_parent(this);
		boxBlue.create(L"Blue text on yellow background", 1, 1,
			0, 0, StaticEx::STYLE | SS_CENTER | SS_CENTERIMAGE);
		layout.define(boxBlue).width(320).height(40)
			.centerHorizontally().constraintTop_toBottomOf(boxRed).marginTop(12);

		boxGreen.color(RgbColor("#006000"));
		boxGreen.backgroundColor(RgbColor("#e0ffe0"));
		boxGreen.set_parent(this);
		boxGreen.create(L"Green text on green background", 1, 1,
			0, 0, StaticEx::STYLE | SS_CENTER | SS_CENTERIMAGE);
		layout.define(boxGreen).width(320).height(40)
			.centerHorizontally().constraintTop_toBottomOf(boxBlue).marginTop(12);

		btnChange.set_parent(this);
		btnChange.create(L"Change text", 1, 1);
		layout.define(btnChange).width(140).height(28)
			.centerHorizontally().constraintTop_toBottomOf(boxGreen).marginTop(20);

		// text() is forwarded to the inner Static
		btnChange.onClick([this](EventData&) {
			boxBlue.text(L"Text is changed");
		});

		// put the whole block in the middle of the example area
		coreLayout.define(layout).gravityCenter();
	}
};
