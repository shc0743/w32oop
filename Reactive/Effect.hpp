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

namespace w32oop::reactive {

	class Observable;
	class Observer;

	namespace internal {
		inline thread_local Observer* current_observer = nullptr;
	}

	class Observable : public w32ReactiveObject {
	public:
		Observable() = default;
		Observable(const Observable&) = delete;
		Observable& operator=(const Observable&) = delete;
		virtual ~Observable() = default;

		void add_observer(Observer* observer) {
			if (!observer) return;
			if (std::find(observers_.begin(), observers_.end(), observer) != observers_.end()) return;
			observers_.push_back(observer);
		}

		void remove_observer(Observer* observer) {
			observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
		}

		void notify_observers();

	private:
		std::vector<Observer*> observers_;
	};

	class Observer : public w32ReactiveObject {
	public:
		Observer() = default;
		Observer(const Observer&) = delete;
		Observer& operator=(const Observer&) = delete;

		virtual ~Observer() {
			for (Observable* source : dependencies_) source->remove_observer(this);
		}

		virtual void on_changed() = 0;

		void begin_tracking() {
			for (Observable* source : dependencies_) source->remove_observer(this);
			dependencies_.clear();
			tracking_ = true;
		}

		void end_tracking() { tracking_ = false; }

		void track(Observable& source) {
			if (!tracking_) return;
			if (std::find(dependencies_.begin(), dependencies_.end(), &source) != dependencies_.end()) return;
			dependencies_.push_back(&source);
			source.add_observer(this);
		}

		[[nodiscard]] std::size_t dependency_count() const noexcept { return dependencies_.size(); }

	private:
		std::vector<Observable*> dependencies_;
		bool tracking_ = false;
	};

	inline void Observable::notify_observers() {
		const std::vector<Observer*> snapshot = observers_;
		for (Observer* observer : snapshot) {
			if (std::find(observers_.begin(), observers_.end(), observer) == observers_.end()) continue;
			observer->on_changed();
		}
	}

	class EffectScope : public w32ReactiveObject {
	public:
		explicit EffectScope(Observer* observer) : previous_(internal::current_observer) {
			internal::current_observer = observer;
			if (observer) observer->begin_tracking();
		}

		~EffectScope() {
			if (internal::current_observer) internal::current_observer->end_tracking();
			internal::current_observer = previous_;
		}

		EffectScope(const EffectScope&) = delete;
		EffectScope& operator=(const EffectScope&) = delete;

	private:
		Observer* previous_ = nullptr;
	};

	inline void track(Observable& source) {
		if (Observer* observer = internal::current_observer) observer->track(source);
	}
}
