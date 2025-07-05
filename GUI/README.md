# w32oop/GUI

A set of utilities to simplify GUI programming.

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

## More examples

[Examples](../.examples/GUI/)

# License

MIT
