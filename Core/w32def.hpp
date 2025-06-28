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

#include <string>
#include <stdexcept>
#include <utility>
#include <map>
#include <functional>
#include <mutex>
#include <windows.h>
#include <windowsx.h>

#define w32oop_declare_exception_class(name) class name##_exception : public w32oop_exception { public: name##_exception(string d) : w32oop_exception(d) {} name##_exception() : w32oop_exception(( "Exception: " # name )) {} }

#define w32oop_declare_exception_class_from(name, _Ty) class name##_exception : public _Ty { public: name##_exception(string d) : _Ty(d) {} name##_exception() : _Ty(( "Exception: " # name )) {} }

namespace w32oop {
    using namespace std;
    
    namespace core {
        // The w32Object class is the base class for all w32oop objects.
        // All classes in `w32oop` are inherited from this class.
        class w32Object {
        public:
            w32Object() = default;
            virtual ~w32Object() = default;
        };
    }

    namespace def {
        // Interface class for RAII objects.
        class w32RAIIObject : public core::w32Object {
        public:
            w32RAIIObject() = default;
            virtual ~w32RAIIObject() = default;
        };
    }

    namespace exceptions {
#define declare_exception(name) class name##_exception : public runtime_error { public: name##_exception(string d) : runtime_error(d) {} name##_exception() : runtime_error(( "Exception: " # name )) {} }
        declare_exception(w32oop);
#undef declare_exception

    }

    using core::w32Object;
    using namespace def;
    using exceptions::w32oop_exception;
}

#include "./publicdef.hpp"

