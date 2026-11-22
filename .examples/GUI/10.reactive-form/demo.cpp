// w32oop Framework Example
//
// : 11.reactive-form/demo.cpp
// Declarative + reactive UI: the window owns no control members

#include <w32use.hpp>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;
using namespace w32oop::reactive;

static_assert(HNode<foundation::Edit>);
static_assert(HNode<FlexLayout>);
static_assert(!HNode<int>);

class ReactiveFormWindow : public Window {
public:
	ReactiveFormWindow() : Window(L"Reactive Form Demo", 640, 440, 0, 0, WS_OVERLAPPEDWINDOW) {}

private:
	Ref<std::wstring> hex{ L"#3399ff" };
	Ref<std::wstring> caption{ L"Hello, reactive world" };
	COLORREF last_valid_color = RGB(0x33, 0x99, 0xff);

	Computed<COLORREF> color = computed([this]() {
		try {
			last_valid_color = ParseRgbColorRt(w32oop::util::str::converts::wstr_str(hex.value()));
		}
		catch (const w32oop::exceptions::invalid_rgb_value_exception&) {
			// Keep the previous valid color if the input is not completed
		}
		return last_valid_color;
	});
	Computed<std::wstring> summary = computed([this]() {
		return L"color = " + to_hex(color.value()) + L", caption length = " + std::to_wstring(caption.value().size());
	});

	WatchHandle title_watcher;
	LayoutManager layout;

	static std::wstring to_hex(COLORREF color) {
		wchar_t buffer[16]{};
		swprintf_s(buffer, L"#%02x%02x%02x", GetRValue(color), GetGValue(color), GetBValue(color));
		return buffer;
	}

	void onCreated() override {
		title_watcher = watch(hex, [this](const std::wstring& value, const std::wstring&) {
			text(L"Reactive Form Demo - " + value);
		});

		layout.setRenderer([this]() {
			return h<FlexLayout>({ { "flex-direction", "column" }, { "gap", 12 }, { "padding", 16 } }, {
				h<StaticEx>({
					{ "style", { { "color", color }, { "background-color", RGB(0xf7, 0xf7, 0xf7) } } },
					{ "height", 56 }
				}, caption),

				h<Edit>({ { "w-model:value", hex.toRef() }, { "height", 30 } }, nullptr),
				h<Edit>({ { "w-model:value", caption.toRef() }, { "height", 30 } }, nullptr),

				h<Static>({ { "value", summary.value() }, { "height", 24 } }, nullptr),

				h<FlexLayout>({ { "flex-direction", "row" }, { "gap", 8 }, { "height", 34 } }, {
					h<Button>({ { "value", L"Reset" }, { "width", 120 }, { "@click", [this]() {
						hex = L"#3399ff";
						caption = L"Hello, reactive world";
					} } }, nullptr),
					h<Button>({ { "value", L"Red" }, { "width", 120 }, { "@click", [this]() {
						hex = L"#ff3333";
					} } }, nullptr),
				}),

				h<Static>({ { "value", L"Color field accepts #RGB, #RRGGBB or rgb(r,g,b). Both edits are two-way bound." },
					{ "height", 22 } }, nullptr),
			});
		});

		setLayoutManager(&layout);
	}

	void onDestroy() override {
		title_watcher.stop();
	}

protected:
	void setup_event_handlers() override {}
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

	ReactiveFormWindow window;
	window.create();
	window.set_main_window();
	window.center();
	window.show(nCmdShow);
	return window.run();
}
