#pragma once
#include "../../../w32use.hpp"

// TrackBar example: range, ticks and the position notification.
class TrackBarExample : public ChildControlWindow {
public:
	TrackBarExample(HWND parent) : ChildControlWindow(parent, L"TrackBar example", 1, 1) {}

protected:
	LinearLayout coreLayout;
	ConstraintLayout layout;
	TrackBar track;
	Static lbl;

	void setup_event_handlers() override {
		setLayoutManager(&coreLayout);
	}

	void onCreated() override {
		track.set_parent(this);
		track.create(L"", 1, 1);
		track.set_range(0, 100);
		track.set_tick_frequency(10);
		track.pos(50);
		layout.define(track).width(320).height(45)
			.centerHorizontally().constraintTop_toTopOf(nullptr);

		lbl.set_parent(this);
		lbl.create(L"Position: 50", 1, 1);
		layout.define(lbl).width(260).height(24)
			.centerHorizontally().constraintTop_toBottomOf(track).marginTop(12);

		// TRBN_THUMBPOSCHANGING: dwPos is the position the thumb is moving to
		track.onbeforechange([this](EventData& ev) {
			auto* info = reinterpret_cast<NMTRBTHUMBPOSCHANGING*>(ev.lParam);
			lbl.text(L"Position: " + std::to_wstring(info->dwPos));
		});

		coreLayout.define(layout).gravityCenter();
	}
};
