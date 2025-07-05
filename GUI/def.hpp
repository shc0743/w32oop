#pragma once
/*
MIT License, Copyright (c) 2025 @chcs1013
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef __cplusplus
#error "Must be included in C++"
#endif

#include "../Core/w32def.hpp"

namespace w32oop::def {
    class w32GUIObject : public w32Object {
    public:
        w32GUIObject() = default;
        virtual ~w32GUIObject() = default;
    };
    class w32Window : public w32GUIObject {
    public:
        w32Window() = default;
        virtual ~w32Window() = default;
    };
}

namespace w32oop::exceptions {
    w32oop_declare_exception_class(ui);
    w32oop_declare_exception_class_from(window, ui_exception);
    w32oop_declare_exception_class_from(window_not_initialized, window_exception);
    w32oop_declare_exception_class_from(window_already_initialized, window_exception);
    w32oop_declare_exception_class_from(window_illegal_state, window_exception);
    w32oop_declare_exception_class_from(window_class_registration_failure, window_exception);
    w32oop_declare_exception_class_from(window_creation_failure, window_exception);
    w32oop_declare_exception_class_from(window_has_no_parent, window_exception);
    w32oop_declare_exception_class_from(window_dangerous_thread_operation, window_exception);
    w32oop_declare_exception_class_from(window_hotkey_duplication, window_exception);
}

#include <CommCtrl.h>

namespace w32oop::ui {
    constexpr ULONGLONG WINDOW_NOTIFICATION_CODES = WM_USER + 0x1000FFFFFFFFULL;
    constexpr ULONGLONG WM_MENU_CHECKED = WM_USER + WM_MENUCOMMAND + 0x2000FFFFFFFFULL;

#ifdef ICC_ALL_CLASSES
#undef ICC_ALL_CLASSES
#endif
    constexpr DWORD ICC_ALL_CLASSES =
        ICC_LISTVIEW_CLASSES |
        ICC_TREEVIEW_CLASSES |
        ICC_BAR_CLASSES |
        ICC_TAB_CLASSES |
        ICC_UPDOWN_CLASS |
        ICC_PROGRESS_CLASS |
        ICC_HOTKEY_CLASS |
        ICC_ANIMATE_CLASS |
        ICC_WIN95_CLASSES |
        ICC_DATE_CLASSES |
        ICC_USEREX_CLASSES |
        ICC_COOL_CLASSES |
        ICC_INTERNET_CLASSES |
        ICC_PAGESCROLLER_CLASS |
        ICC_NATIVEFNTCTL_CLASS |
        ICC_STANDARD_CLASSES |
        ICC_LINK_CLASS;
}
