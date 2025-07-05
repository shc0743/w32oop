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

#include "./def.hpp"
#include "./Joinable.hpp"


namespace w32oop::exceptions {
    w32oop_declare_exception_class_from(event_object_operation_failed, concurrency_exception);
}


namespace w32oop::concurrency {
	class EventObject : public Joinable {
    protected:
        w32EventHandle hEvent;
    public:
        EventObject(bool auto_reset = false) : hEvent(CreateEventW(nullptr, !auto_reset, false, nullptr)) {}
        EventObject(bool auto_reset, bool initial_state, PCWSTR name, LPSECURITY_ATTRIBUTES lpsa) :
            hEvent(CreateEventW(lpsa, !auto_reset, initial_state, name)) {}

        ~EventObject() {
            CloseHandle(hEvent);
        }

        // Getters
        HANDLE get() const {
            return hEvent;
        }

        // move constructor
        EventObject(EventObject&& other) : hEvent(move(other.hEvent)) {}

        // move assignment operator
        EventObject& operator=(EventObject&& other) {
            if (this != &other) {
                hEvent = move(other.hEvent);
            }
            return *this;
        }

    public:
        // Operations
        void set();
        void reset();

        virtual void join();

        bool wait(DWORD timeout = INFINITE);

        template <typename Container>
        bool wait_multiple(const Container &events, DWORD timeout = INFINITE, bool wait_all = true);

    };
}

