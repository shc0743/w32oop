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

#include "../def.hpp"
#include "../Layout/FlexLayout.hpp"
#include "../Window.hpp"
#include "./Reconciler.hpp"
#include "./VNode.hpp"
#include "../../Reactive/Effect.hpp"


namespace w32oop::ui {

	class LayoutManager final : public ui::LayoutNode, public Observer {
	public:
		LayoutManager();
		~LayoutManager() override;

		LayoutManager(const LayoutManager&) = delete;
		LayoutManager& operator=(const LayoutManager&) = delete;
		LayoutManager(LayoutManager&&) = delete;
		LayoutManager& operator=(LayoutManager&&) = delete;

		void setRenderer(std::function<VNode()> renderer);
		void renderNow();

		void attach(ui::Window* host);
		void detach();
		void requestLayout();

		[[nodiscard]] ui::Window* find(const std::wstring& key) const;
		[[nodiscard]] std::size_t mounted_count() const;

		[[nodiscard]] ui::LayoutSize measure(ui::LayoutSize available) const override;
		[[nodiscard]] std::vector<ui::Placement> arrange(const ui::LayoutRect& area) override;
		void invalidate() override;
		void set_reflow_handler(std::function<void()> handler) override;

		void on_changed() override;

	private:
		void ensure_rendered();

		ui::Window* host_ = nullptr;
		std::unique_ptr<ui::FlexLayout> root_box_;
		std::unique_ptr<internal::Reconciler> reconciler_;
		std::function<VNode()> renderer_;
		std::function<void()> reflow_handler_;
		bool render_dirty_ = false;
		bool rendering_ = false;
	};
}
