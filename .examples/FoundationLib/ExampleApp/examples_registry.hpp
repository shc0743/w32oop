#pragma once
#include "../../../w32use.hpp"

#pragma region Examples import
#include "ButtonExample.hpp"
#include "EditExample.hpp"
#pragma endregion


using MyExample = std::pair<std::wstring, std::function<std::shared_ptr<Window>(HWND)>>;
static const std::vector<MyExample> Examples{
	MyExample(L"Button", [](HWND parent) { return std::make_shared<ButtonExample>(parent); }),
	MyExample(L"Edit", [](HWND parent) { return std::make_shared<EditExample>(parent); }),
	MyExample(L"Benchmark", [](HWND) { return nullptr; }),
};
