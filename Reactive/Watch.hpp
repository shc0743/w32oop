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
#include "./Computed.hpp"
#include "./Ref.hpp"


namespace w32oop::reactive {
	namespace internal {

		struct WatcherBase;

		inline thread_local int batch_depth = 0;
		inline thread_local std::vector<WatcherBase*> pending_watchers;

		void queue_watcher(WatcherBase* watcher);
		void flush_pending_watchers();

		struct WatcherBase : Observer {
			bool active = true;

			virtual void execute() = 0;

			void on_changed() override {
				if (!active) return;
				if (batch_depth > 0) {
					queue_watcher(this);
					return;
				}
				execute();
			}

			void stop() {
				active = false;
				begin_tracking();
			}
		};

		inline void queue_watcher(WatcherBase* watcher) {
			if (!watcher || !watcher->active) return;
			if (std::find(pending_watchers.begin(), pending_watchers.end(), watcher) != pending_watchers.end()) return;
			pending_watchers.push_back(watcher);
		}

		inline void flush_pending_watchers() {
			if (batch_depth > 0) return;

			int guard = 0;
			while (!pending_watchers.empty() && guard++ < 1000) {
				std::vector<WatcherBase*> batch;
				batch.swap(pending_watchers);
				for (WatcherBase* watcher : batch) {
					if (watcher && watcher->active) watcher->execute();
				}
			}
			pending_watchers.clear();
		}

		template<class T, class F>
		struct TypedWatcher final : WatcherBase {
			TypedWatcher(std::function<T()> getter, F cb)
				: get(std::move(getter)), callback(std::move(cb)) {
			}

			std::function<T()> get;
			F callback;
			T cached{};
			bool has_cached = false;
			bool compare = true;
			bool immediate = false;
			bool first_run = true;
			bool running = false;

			void execute() override {
				if (running) return;
				running = true;

				T now{};
				try {
					EffectScope scope(this);
					now = get();
				}
				catch (...) {
					running = false;
					throw;
				}
				running = false;

				const bool changed = !has_cached || !compare || !(now == cached);
				const T previous = has_cached ? cached : T{};
				cached = now;
				has_cached = true;

				const bool fire = changed && !(first_run && !immediate);
				first_run = false;
				if (!fire) return;

				invoke(now, previous);
			}

			void invoke(const T& now, const T& previous) {
				if constexpr (std::is_invocable_v<F&, const T&, const T&>) {
					callback(now, previous);
				}
				else if constexpr (std::is_invocable_v<F&, const T&>) {
					callback(now);
				}
				else if constexpr (std::is_invocable_v<F&>) {
					callback();
				}
				else {
					static_assert(sizeof(F) == 0,
						"watch callback must accept (new_value, old_value), (value) or no arguments");
				}
			}
		};
	}

	class WatchHandle : public w32ReactiveObject {
	public:
		WatchHandle() = default;
		explicit WatchHandle(std::shared_ptr<internal::WatcherBase> watcher) : watcher_(std::move(watcher)) {}

		void stop() {
			if (watcher_) {
				watcher_->stop();
				watcher_.reset();
			}
		}

		[[nodiscard]] bool active() const noexcept { return watcher_ && watcher_->active; }

	private:
		std::shared_ptr<internal::WatcherBase> watcher_;
	};

	template<class FGet, class F>
	[[nodiscard]] WatchHandle watch_effect(FGet&& getter, F&& callback) {
		using T = std::invoke_result_t<FGet&>;
		auto watcher = std::make_shared<internal::TypedWatcher<T, std::decay_t<F>>>(std::function<T()>(std::forward<FGet>(getter)), std::forward<F>(callback));
		watcher->compare = false;
		watcher->immediate = true;
		watcher->execute();
		return WatchHandle(watcher);
	}

	template<class T, class F>
	[[nodiscard]] WatchHandle watch(const Ref<T>& source, F&& callback) {
		using Value = T;
		auto watcher = std::make_shared<internal::TypedWatcher<Value, std::decay_t<F>>>(std::function<Value()>([source]() { return source.value(); }), std::forward<F>(callback));
		watcher->compare = true;
		watcher->immediate = false;
		watcher->execute();
		return WatchHandle(watcher);
	}

	template<class T, class F>
	[[nodiscard]] WatchHandle watch(const Computed<T>& source, F&& callback) {
		using Value = T;
		auto watcher = std::make_shared<internal::TypedWatcher<Value, std::decay_t<F>>>(std::function<Value()>([source]() { return source.value(); }), std::forward<F>(callback));
		watcher->compare = true;
		watcher->immediate = false;
		watcher->execute();
		return WatchHandle(watcher);
	}

	template<class F>
	void batch(F&& fn) {
		struct Guard {
			Guard() { ++internal::batch_depth; }
			~Guard() {
				if (--internal::batch_depth == 0) internal::flush_pending_watchers();
			}
			Guard(const Guard&) = delete;
			Guard& operator=(const Guard&) = delete;
		} guard;

		std::forward<F>(fn)();
	}
}
