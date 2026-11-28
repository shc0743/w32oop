#pragma once
#include "../../../w32use.hpp"

#pragma region Examples import
#include "StaticExample.hpp"
#include "StaticExExample.hpp"
#include "ButtonExample.hpp"
#include "EditExample.hpp"
#include "CheckBoxExample.hpp"
#include "ListBoxExample.hpp"
#include "TrackBarExample.hpp"
#include "StatusBarExample.hpp"
#pragma endregion


using MyExample = std::pair<std::wstring, std::function<std::shared_ptr<Window>(HWND)>>;
static const std::vector<MyExample> Examples{
	MyExample(L"Static", [](HWND parent) { return std::make_shared<StaticExample>(parent); }),
	MyExample(L"StaticEx", [](HWND parent) { return std::make_shared<StaticExExample>(parent); }),
	MyExample(L"Button", [](HWND parent) { return std::make_shared<ButtonExample>(parent); }),
	MyExample(L"Edit", [](HWND parent) { return std::make_shared<EditExample>(parent); }),
	MyExample(L"CheckBox", [](HWND parent) { return std::make_shared<CheckBoxExample>(parent); }),
	MyExample(L"ListBox", [](HWND parent) { return std::make_shared<ListBoxExample>(parent); }),
	MyExample(L"TrackBar", [](HWND parent) { return std::make_shared<TrackBarExample>(parent); }),
	MyExample(L"StatusBar", [](HWND parent) { return std::make_shared<StatusBarExample>(parent); }),
	MyExample(L"---", [](HWND) { return nullptr; }),
	MyExample(L"Benchmark", [](HWND hwnd) {
		if (IDOK != MessageBoxW(hwnd, L"Are you sure? This will create a lot of windows!!!",
			L"Warning", MB_ICONQUESTION | MB_OKCANCEL)) return nullptr;
		std::vector<std::shared_ptr<Window>> ws;
		size_t s = 0;
		for (size_t i = 0; i < 1000; ++i) try {
			auto w = std::make_shared<EditExample>(hwnd);
			w->create();
			w->show();
			ws.push_back(w);
			s = ws.size();
		}
		catch (std::exception& exc) {
			s = ws.size();
			ws.clear();
			MessageBoxW(hwnd, std::format(L"Created {} windows then failed at #{}!\r\nError: {}", s, i + 1,
				w32oop::util::str::converts::str_wstr(exc.what())).c_str(),
				L"Benchmark", MB_ICONINFORMATION);
			break;
		}
		MessageBoxW(hwnd, std::format(L"Created {} windows", s).c_str(), L"Success", MB_ICONINFORMATION);
		return nullptr;
	}),
	MyExample(L"---", [](HWND) { return nullptr; }),
	MyExample(L"Quit", [](HWND hwnd) {
		if (IDOK == MessageBoxW(hwnd, L"Are you sure?", L"Quit", MB_ICONQUESTION | MB_OKCANCEL))
			PostMessageW(hwnd, WM_CLOSE, 0, 0);
		return nullptr;
	}),
};
