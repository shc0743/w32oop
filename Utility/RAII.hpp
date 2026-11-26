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

namespace w32oop::exceptions {
	w32oop_declare_exception_class_from(raii, utility_exception);

	w32oop_declare_exception_class_from(srw_lock_guard_invalid_handle, raii_exception);
	w32oop_declare_exception_class_from(srw_lock_guard_mode_not_set, raii_exception);
	w32oop_declare_exception_class_from(srw_lock_guard_not_locked, raii_exception);
	w32oop_declare_exception_class_from(srw_lock_guard_already_locked, raii_exception);
}

namespace w32oop::util {
	class RAIIHelper : public w32RAIIObject {
	private:
		std::function<void()> fp;
	public:
		RAIIHelper(std::function<void()>f) : fp(f) {}
		~RAIIHelper() { fp(); }
	};
	using WindowRAIIHelper = RAIIHelper; // For compatibility with old code
	class SRWLockGuard final : public w32RAIIObject {
	public:
		enum Mode {
			Unknown,
			Exclusive,
			Shared,
		};
	private:
		SRWLOCK* lpLock;
		Mode mode;
		bool locked = false;
	protected:
		void _check() const {
			if (!lpLock) throw exceptions::srw_lock_guard_invalid_handle_exception();
		}
	public:
		// initialize the guard without acquiring
		SRWLockGuard(SRWLOCK* lpSrwLock) : lpLock(lpSrwLock), mode(Unknown) {
			_check();
		}
		// initialize the guard and immediately acquire the lock, mode must be valid
		SRWLockGuard(SRWLOCK* lpSrwLock, Mode myMode) : lpLock(lpSrwLock), mode(myMode) {
			_check();
			acquire();
		}
		// do not copy the guard
		SRWLockGuard(const SRWLockGuard&) = delete;
		SRWLockGuard& operator=(const SRWLockGuard&) = delete;
		// do not move the guard
		SRWLockGuard(SRWLockGuard&&) = delete;
		SRWLockGuard& operator=(SRWLockGuard&&) = delete;
		// automatically release
		~SRWLockGuard() {
			if (locked) release();
		}
		void acquire() {
			if (locked) throw exceptions::srw_lock_guard_already_locked_exception();
			_Analysis_assume_lock_not_held_(lpLock);
			switch (mode) {
			case Exclusive:
				AcquireSRWLockExclusive(lpLock);
				break;
			case Shared:
				AcquireSRWLockShared(lpLock);
				break;
			default:
				throw exceptions::srw_lock_guard_mode_not_set_exception();
			}
			locked = true;
		}
		void acquire(Mode myMode) {
			if (locked) throw exceptions::srw_lock_guard_already_locked_exception();
			mode = myMode;
			acquire();
		}
		bool isAcquired() const noexcept {
			return locked;
		}
		Mode currentMode() const noexcept {
			return mode;
		}
		void release() {
			if (!locked) throw exceptions::srw_lock_guard_not_locked_exception();
			_Analysis_assume_lock_held_(lpLock);
			switch (mode) {
			case Exclusive:
				ReleaseSRWLockExclusive(lpLock);
				break;
			case Shared:
				ReleaseSRWLockShared(lpLock);
				break;
			default:
				throw exceptions::srw_lock_guard_mode_not_set_exception();
			}
			locked = false;
		}
	};
}
