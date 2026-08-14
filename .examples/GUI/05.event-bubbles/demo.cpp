// w32oop Framework Example
//
// : 05.event-bubbles/demo.cpp
// This example demonstrates how to use event bubbling in the GUI framework.

#include <w32use.hpp>
#include <algorithm>
#include <random>
#include <format>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;

static long long fib(int n);
static long long quick_pow(long long base, long long exp);
static int binary_search(const vector<int>& arr, int target);
static void bubble_sort(vector<int>& arr);

namespace MyDemo {
	class EventBubbleDemo : public Window {
	protected:
		Button btn1, btn2, btn3, btn4;

	public:
		EventBubbleDemo(const wstring& title, int width, int height, int x = 0, int y = 0)
			: Window(title, width, height, x, y, WS_OVERLAPPEDWINDOW)
		{}
	protected:
		void onCreated() override {
			btn1.set_parent(this);
			btn1.create(L"Button1", 60, 30, 10, 10);
			btn2.set_parent(this);
			btn2.create(L"Button2", 60, 30, 10, 50);
			btn3.set_parent(this);
			btn3.create(L"Button3", 60, 30, 10, 90);
			btn4.set_parent(this);
			btn4.create(L"Button4", 60, 30, 10, 130);

			// Bubble:
			addEventListener(WINDOW_NOTIFICATION_CODES + BN_CLICKED, [this](EventData& event) {
				if (!dynamic_cast<Button*>(event.source())) return; // check if it's a button
				MessageBoxW(hwnd, (L"Button: " + event.source()->text()).c_str(), L"Bubbled to Parent Window", MB_ICONINFORMATION);
			});

			// Button1: Doesn't handle; the event will bubble to the parent window.

			// Button2: Handles the event *without* calling `stopPropagation()`.
			//  The event will bubble to the parent window.
			btn2.onClick([this](EventData& event) {
				MessageBoxW(hwnd, L"Button2 Handler", L"Button2", MB_ICONINFORMATION);
			});

			// Button3: Handles the event *with* `stopPropagation()`.
			//  The event will **NOT** bubble to the parent window.
			btn3.onClick([this](EventData& event) {
				MessageBoxW(hwnd, L"Button3 Handler", L"Button3", MB_ICONINFORMATION);
				event.stopPropagation();
			});

			// Button4: Some algorithm example.
			btn4.onClick([this](EventData& event) {
				event.preventDefault();
				event.stopPropagation();
				random_device rd;
				mt19937 rng(rd());
				uniform_int_distribution<int> dist(0, 3);
				int op = dist(rng);
				wstring out;
				if (op == 0) {
					auto val = fib(42);
					out = format(L"[1] Result: {}", val);
				}
				else if (op == 1) {
					auto val = quick_pow(7, 13);
					out = format(L"[2] Result: {}", val);
				}
				else if (op == 2) {
					vector<int> arr{ 3,7,12,19,24,33,41,56,78,91 };
					auto val = binary_search(arr, 41);
					out = format(L"[3] Result: {}", val);
				}
				else {
					vector<int> arr{ 51,22,87,13,9,44,72,3 };
					bubble_sort(arr);
					out = format(L"[4] Result: {}", arr[3]);
				}
				MessageBoxW(hwnd, out.c_str(), L"Result", MB_ICONINFORMATION);
			});
		}
		void onClicked(EventData&) {
			MessageBoxW(hwnd, L"Window clicked!", L"Notification", MB_OK);
		}
	private:
		virtual void setup_event_handlers() override {
			WINDOW_add_handler(WM_LBUTTONDOWN, onClicked);
		}
	};
}

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR pCmdLine,
	_In_ int nCmdShow)
{
	// create the application
	MyDemo::EventBubbleDemo app(L"Event Bubble Demo", 640, 480);
	// create it
	app.create();
	// set the main window
	app.set_main_window();
	// center
	app.center();
	// show it
	app.show();
	// message loop
	return app.run();
}


#pragma region Algorithm demos

static long long fib(int n)
{
	if (n <= 1) return n;
	long long a = 0, b = 1;
	for (int i = 2; i <= n; ++i)
	{
		long long c = a + b;
		a = b;
		b = c;
	}
	return b;
}

static long long quick_pow(long long base, long long exp)
{
	long long res = 1;
	while (exp > 0)
	{
		if (exp & 1) res *= base;
		base *= base;
		exp >>= 1;
	}
	return res;
}

static int binary_search(const vector<int>& arr, int target)
{
	int l = 0, r = static_cast<int>(arr.size()) - 1;
	while (l <= r)
	{
		int mid = (l + r) / 2;
		if (arr[mid] == target) return mid;
		else if (arr[mid] < target) l = mid + 1;
		else r = mid - 1;
	}
	return -1;
}

static void bubble_sort(vector<int>& arr)
{
	size_t n = arr.size();
	for (size_t i = 0;i < n;i++) {
		for (size_t j = 0;j < n - i - 1;j++) {
			if (arr[j] > arr[j + 1]) {
				swap(arr[j], arr[j + 1]);
			}
		}
	}
}
#pragma endregion

