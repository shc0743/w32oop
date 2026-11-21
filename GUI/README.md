# w32oop/GUI

A set of utilities to simplify GUI programming.

### Recent Breaking Changes

We now disable the `force_focus` by default in order to ease the VirusTotal wrong detection in commit `d84ac02ec754de737cb28b6d4278d4953b99a684`.

[Wrong detection](https://www.virustotal.com/gui/file/5d1e3e8e9d1013e6c07907b731311b16a8d549528599518b7706c499d9066cef)
[Wrong detection](https://www.virustotal.com/gui/file/86a12cdd8171a17958de56f9f4465eba1a9800644f4341d99d9832191329631a)
[Wrong detection](https://www.virustotal.com/gui/file/33c1b70e809cada9f43801b1568e08b028441e2714f8bab888f034e13b5cda9c)

```cpp
bool Window::force_focus(DWORD timeout) {
#ifdef W32OOP_GUI_USE_FORCE_FOCUS
// Previous Code
#endif
	return focus();
}
```

Obviously, if you have to use `force_focus`, you should define the `W32OOP_GUI_USE_FORCE_FOCUS` in your project configuration.

## Usage

1. Include the `<w32use.hpp>` in your project
2. Extend the `Window` class

## What to do?

**Override the following function**

```cpp
virtual void setup_event_handlers() override {
    WINDOW_add_handler(WM_YourMessage, onYourMessage);
}
void onYourMessage(EventData& event) {
    // Your code here
    ...
    // You can prevent the default handler
    // Default handler is DefWindowProc
    event.preventDefault();
}
// **Optional**: if you want to customize the window class
// We recommend not to override it manually
// since the framework use RTTI to generate
// the window class automatically
const wstring get_class_name() const override {
    return L"MyWndClass";
}
```

**Creating controls**

```cpp
class YourWindowClass : public Window {
public:
    YourWindowClass() : Window(L"Your Window Class", 400, 300, 0, 0, WS_OVERLAPPEDWINDOW) {}
private:
    Static text;
    Button btn;
    Edit textBox;
    void onCreated() override {
        // lifecycle hooks
        text.set_parent(this); // Must call
        text.create(L"...", 250, 30, 10, 90);

        btn.set_parent(this); // Must call
        btn.create(L"Click me!", 100, 30, 10, 10);
        btn.onClick([this] (EventData& event) {
            text.text(L"Hello, World!");
        });

        textBox.set_parent(this); // Must call
        textBox.create(L"Hello, World!", 200, 30, 10, 50);
        textBox.onChange([this] (EventData& event) {
            text.text(L"Text area edited");
        });
    }
    void onDestroy() override {
        // lifecycle hooks
        // you can do your cleanup here
    }
protected:
    virtual void setup_event_handlers() override {

    }
};

int main() { 
    YourWindowClass window;
    window.create();
    window.set_main_window();
    window.center();
    window.show();
    return window.run();
}
```

[Full Demo Here](../.examples/GUI/10.creating-controls/demo.cpp)

## Layout (flexbox)

`FlexLayout` is a one-dimensional flexbox engine. Bind a root layout to a window
with `set_layout()`; a nested `FlexLayout` is pure geometry (it has no HWND).

```cpp
class MyWindow : public Window {
    FlexLayout root{Direction::Column};
    FlexLayout toolbar{Direction::Row};
    Edit editor;
    Button ok, cancel;

    void setup_event_handlers() override {
        set_layout(root);
    }

    void onCreated() override {
        // Create the controls first (parent = *this) ...
        root.padding(12).gap(8);
        root.add(toolbar).height(32);
        toolbar.gap(8);
        toolbar.add(ok).width(96);
        toolbar.add(cancel).width(96);
        root.add(editor).grow(1).basis(0).minHeight(0);  // takes the rest
    }
};
```

* Container properties mirror CSS: `direction`, `wrap`, `justifyContent`,
  `alignItems`, `alignContent`, `gap` / `rowGap` / `columnGap`, `padding`.
* Item properties: `grow`, `shrink`, `basis`, `width` / `height` (pixels or
  `Percent{..}`), `minWidth` / `maxWidth` / `minHeight` / `maxHeight`, `margin`,
  `alignSelf`.
* `grow(1).basis(0)` is CSS `flex: 1`; `grow(1)` alone grows from the content size.
* Items keep insertion order. `remove()` / `clear()` update the tree.
* The window reflows automatically on `WM_SIZE` and whenever the tree changes;
  `relayout()` forces it manually.
* The content size of a control is the size passed to its constructor. Custom
  controls can override `Window::content_size()`.
* Differences from CSS: minimum size defaults to `0` (no `min-width: auto` trap),
  no auto margins, no `order`, no baseline alignment, no RTL, no text metrics.
  Overflow is clipped by the parent window.

The flexbox solver is headless and unit tested:

```bat
cmake --preset vs2026-x64 -DW32OOP_BUILD_TESTS=ON
cmake --build --preset vs2026-x64-release
ctest --test-dir build/vs2026-x64 -C Release --output-on-failure
```

[Full Demo Here](../.examples/GUI/10.use-layouts/demo.cpp)

## More examples

[Examples](../.examples/GUI/)

# License

MIT
