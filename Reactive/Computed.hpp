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
#include "./Effect.hpp"
#include <type_traits>

namespace w32oop::reactive {

	template<class T>
	class Computed : public w32ReactiveObject {
	private:
		struct State final : Observer, Observable {
			explicit State(std::function<T()> fn) : fn(std::move(fn)) {}

			std::function<T()> fn;
			mutable T cached{};
			mutable bool dirty = true;
			mutable bool computing = false;

			[[nodiscard]] const T& get() const {
				if (!dirty) return cached;
				if (computing) throw exceptions::computed_cycle_exception{ "computed<T>: cyclic dependency" };

				computing = true;
				EffectScope scope(const_cast<State*>(this));
				try {
					cached = fn();
				}
				catch (...) {
					computing = false;
					throw;
				}
				computing = false;
				dirty = false;
				return cached;
			}

			void on_changed() override {
				if (dirty) return;
				dirty = true;
				notify_observers();
			}
		};

	public:
		Computed(const Computed&) = default;
		Computed(Computed&&) noexcept = default;
		Computed& operator=(const Computed&) = default;
		Computed& operator=(Computed&&) noexcept = default;

		template<class F>
			requires (!std::same_as<std::remove_cvref_t<F>, Computed>)
		explicit Computed(F&& fn)
			: state_(std::make_shared<State>(std::function<T()>(std::forward<F>(fn)))) {
		}

		[[nodiscard]] const T& value() const {
			track(static_cast<Observable&>(*state_));
			return state_->get();
		}

		operator T() const { return value(); }

	private:
		std::shared_ptr<State> state_;
	};

	template<class F>
	[[nodiscard]] auto computed(F&& fn) -> Computed<std::invoke_result_t<F&>> {
		return Computed<std::invoke_result_t<F&>>(std::forward<F>(fn));
	}
}
