#pragma once
#include "../../../w32use.hpp"

class ButtonExample : public ChildControlWindow {
public:
	ButtonExample(HWND parent) : ChildControlWindow(parent, L"Button example", 1, 1) {}

protected:
	ConstraintLayout layout;
	Button btn;
	void setup_event_handlers() override {
		setLayoutManager(&layout);
	}
	void onCreated() override {
		btn.set_parent(this);
		btn.create(L"Button", 1, 1);
		layout.define(btn).centerInParent().width(100).height(30);
		btn.onClick([this](EventData&) {
			MessageBoxW(hwnd, L"Button is clicked!!", text().c_str(), MB_ICONINFORMATION);
		});
	}
};

