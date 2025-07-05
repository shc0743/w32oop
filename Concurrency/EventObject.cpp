#include "./EventObject.hpp"

void w32oop::concurrency::EventObject::set() {
    if (!SetEvent(hEvent)) {
        throw w32oop::exceptions::event_object_operation_failed_exception("Failed to set event object.");
    }
}

void w32oop::concurrency::EventObject::reset() {
    if (!ResetEvent(hEvent)) {
        throw w32oop::exceptions::event_object_operation_failed_exception("Failed to reset event object.");
    }
}

void w32oop::concurrency::EventObject::join() {
    if (!wait(INFINITE)) {
        throw w32oop::exceptions::event_object_operation_failed_exception("Failed to join event object.");
    }
}

bool w32oop::concurrency::EventObject::wait(DWORD timeout) {
    DWORD result = WaitForSingleObject(hEvent, timeout);
    if (result == WAIT_FAILED) {
        throw w32oop::exceptions::event_object_operation_failed_exception("Failed to wait for event object.");
    }
    return (result == WAIT_OBJECT_0);
}

