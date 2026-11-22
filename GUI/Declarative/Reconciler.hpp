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
#include "./VNode.hpp"


namespace w32oop::ui::internal {

	class EventSinkImpl;
	struct NodeState;

	class Reconciler {
	public:
		Reconciler(ui::Window& host, ui::FlexLayout& root_box);
		~Reconciler();

		Reconciler(const Reconciler&) = delete;
		Reconciler& operator=(const Reconciler&) = delete;

		void render(const VNode& tree);

		[[nodiscard]] ui::Window* find(const std::wstring& key) const;
		[[nodiscard]] std::size_t mounted_count() const;

	private:
		std::unique_ptr<NodeState> mount_node(const VNode& vnode, bool is_root = false);
		void patch_node(NodeState& node, const VNode& next);
		void sync_layout(NodeState& node) const;
		void apply_widget_props(NodeState& node, const VNode& next, const VNode* previous);
		void rebuild_events(NodeState& node, const VNode& next);
		void release_node(NodeState& node) const;
		void collect_count(const NodeState& node, std::size_t& count) const;
		ui::Window* find_in(const NodeState& node, const std::wstring& key) const;

		ui::Window& host_;
		ui::FlexLayout& root_box_;
		std::unique_ptr<NodeState> root_;
		std::vector<std::unique_ptr<NodeState>> graveyard_;
	};
}
