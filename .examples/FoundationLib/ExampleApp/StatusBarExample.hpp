#pragma once
#include "../../../w32use.hpp"

// StatusBar example: a status bar with three parts, docked at the bottom.
class StatusBarExample : public ChildControlWindow {
public:
	StatusBarExample(HWND parent) : ChildControlWindow(parent, L"StatusBar example", 1, 1) {}

protected:
	ConstraintLayout layout;
	StatusBar bar;
	Static lbl;
	Button btn;
	int clicks = 0;

	void setup_event_handlers() override {
		setLayoutManager(&layout);
	}

	void onCreated() override {
		// dock the status bar to the bottom edge, full width
		bar.set_parent(this);
		bar.create(L"", 1, 1);
		layout.define(bar).matchParent().height(24)
			.constraintStart_toStartOf(LayoutRef{})
			.constraintEnd_toEndOf(LayoutRef{})
			.constraintBottom_toBottomOf(LayoutRef{});

		// SB_SETPARTS takes the right edge of each part (logical pixels here,
		// set_parts scales them for the DPI virtualization)
		int edges[3] = { 150, 370, -1 };
		bar.set_parts(3, edges);
		bar.set_text(0, L"Ready");
		bar.set_text(1, L"Clicks: 0");
		bar.set_text(2, L"Part 2");

		lbl.set_parent(this);
		lbl.create(L"The status bar has 3 parts.", 1, 1);
		layout.define(lbl).width(400).height(24)
			.centerHorizontally().constraintBottom_toTopOf(btn).marginBottom(16);

		btn.set_parent(this);
		btn.create(L"Change part 1", 1, 1);
		layout.define(btn).width(180).height(28).centerInParent();

		btn.onClick([this](EventData&) {
			bar.set_text(1, L"Clicks: " + std::to_wstring(++clicks));
		});
	}
};
