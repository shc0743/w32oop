#include "./Reconciler.hpp"
#include "./Diff.hpp"
#include "./LayoutProperties.hpp"
#include "./Widget.hpp"

using namespace std;
using namespace w32oop;
using namespace w32oop::ui;

namespace {
	using w32oop::ui::EventPayload;
	using w32oop::ui::EventHandler;
	using w32oop::ui::EventSink;
	using w32oop::ui::PropertyValue;
	using w32oop::ui::RefHandle;
	using w32oop::ui::VNode;
	using w32oop::ui::WidgetDescriptor;

	void debug_warn(const std::wstring& text) {
		if (ui::Window::get_global_option(ui::Window::Option_DebugMode) == 0) return;
		std::wstring line = L"[w32oop::ui] ";
		line += text;
		line += L"\n";
		OutputDebugStringW(line.c_str());
	}

	const PropertyValue* find_prop(const VNode& node, std::wstring_view name) {
		for (const auto& entry : node.props) {
			if (entry.first.view() == name) return &entry.second;
		}
		return nullptr;
	}

	std::wstring prop_string(const VNode& node, std::wstring_view name) {
		if (const PropertyValue* value = find_prop(node, name)) return value->asString();
		return {};
	}

	bool is_event_prop(std::wstring_view name) {
		return !name.empty() && name.front() == L'@';
	}

	bool is_model_prop(std::wstring_view name) {
		return name.rfind(L"w-model:", 0) == 0;
	}

	const VNode* first_text_child(const VNode& node) {
		for (const VNode& child : node.children) {
			if (child.is_text) return &child;
		}
		return nullptr;
	}
}

namespace w32oop::ui::internal {

	class EventSinkImpl final : public EventSink {
	public:
		void on(std::wstring_view event, EventHandler handler) override {
			handlers_[std::wstring(event)].push_back(std::move(handler));
		}

		void emit(std::wstring_view event, const EventPayload& payload) override {
			const auto it = handlers_.find(std::wstring(event));
			if (it == handlers_.end()) return;

			const std::vector<EventHandler> snapshot = it->second;
			for (const EventHandler& handler : snapshot) {
				if (handler) handler(payload);
			}
		}

		void clear() { handlers_.clear(); }

	private:
		std::map<std::wstring, std::vector<EventHandler>> handlers_;
	};

	struct NodeState {
		const WidgetDescriptor* descriptor = nullptr;
		std::wstring key;
		VNode vnode;

		std::shared_ptr<EventSinkImpl> sink;
		RefHandle ref_handle;
		RefHandle model_handle;

		std::unique_ptr<ui::Window> control;
		std::unique_ptr<ui::FlexLayout> owned_box;
		ui::FlexLayout* box = nullptr;

		std::vector<std::unique_ptr<NodeState>> children;
		bool suppress_events = false;
	};

	Reconciler::Reconciler(ui::Window& host, ui::FlexLayout& root_box)
		: host_(host), root_box_(root_box) {
	}

	Reconciler::~Reconciler() = default;

	void Reconciler::render(const VNode& tree) {
		if (!root_) root_ = mount_node(tree, true);
		else patch_node(*root_, tree);

		sync_layout(*root_);
		graveyard_.clear();
	}

	std::unique_ptr<NodeState> Reconciler::mount_node(const VNode& vnode, bool is_root) {
		auto node = std::make_unique<NodeState>();
		node->descriptor = descriptor_of(vnode);
		if (!node->descriptor) {
			throw std::runtime_error("w32oop::ui: VNode has no widget descriptor");
		}
		node->key = prop_string(vnode, L"key");
		node->vnode = vnode;

		if (node->descriptor->is_container) {
			if (is_root) {
				node->box = &root_box_;
			}
			else {
				node->owned_box = std::make_unique<ui::FlexLayout>();
				node->box = node->owned_box.get();
			}
			for (const VNode& child : vnode.children) {
				if (child.is_text) {
					debug_warn(L"text child inside a layout container is ignored");
					continue;
				}
				node->children.push_back(mount_node(child));
			}
		}
		else {
			node->sink = std::make_shared<EventSinkImpl>();
			node->control = node->descriptor->create(host_);
			if (!node->control) {
				throw std::runtime_error("w32oop::ui: widget factory returned null");
			}
			node->descriptor->bind_events(*node->control, *node->sink);
			apply_widget_props(*node, vnode, nullptr);
			rebuild_events(*node, vnode);
		}

		return node;
	}

	void Reconciler::patch_node(NodeState& node, const VNode& next) {
		const VNode previous = std::move(node.vnode);
		node.vnode = next;
		node.key = prop_string(next, L"key");

		if (node.control) {
			apply_widget_props(node, next, &previous);
			rebuild_events(node, next);
			return;
		}

		std::vector<const VNode*> next_children;
		for (const VNode& child : next.children) {
			if (!child.is_text) next_children.push_back(&child);
		}

		std::vector<std::wstring> old_keys, new_keys;
		std::vector<std::type_index> old_types, new_types;
		old_keys.reserve(node.children.size());
		old_types.reserve(node.children.size());
		for (const auto& child : node.children) {
			old_keys.push_back(child->key);
			old_types.push_back(child->descriptor->type);
		}
		new_keys.reserve(next_children.size());
		new_types.reserve(next_children.size());
		for (const VNode* child : next_children) {
			new_keys.push_back(prop_string(*child, L"key"));
			new_types.push_back(child->type);
		}

		const ChildPlan plan = plan_children(old_keys, old_types, new_keys, new_types);

		std::vector<std::unique_ptr<NodeState>> old = std::move(node.children);
		node.children.clear();

		for (std::size_t i = 0; i < next_children.size(); ++i) {
			if (plan.reuse[i] >= 0) {
				auto child = std::move(old[static_cast<std::size_t>(plan.reuse[i])]);
				patch_node(*child, *next_children[i]);
				node.children.push_back(std::move(child));
			}
			else {
				node.children.push_back(mount_node(*next_children[i]));
			}
		}

		for (int index : plan.unmount) {
			if (index < 0 || static_cast<std::size_t>(index) >= old.size()) continue;
			if (!old[static_cast<std::size_t>(index)]) continue;
			release_node(*old[static_cast<std::size_t>(index)]);
			graveyard_.push_back(std::move(old[static_cast<std::size_t>(index)]));
		}

		for (auto& leftover : old) {
			if (!leftover) continue;
			release_node(*leftover);
			graveyard_.push_back(std::move(leftover));
		}
	}

	void Reconciler::apply_widget_props(NodeState& node, const VNode& next, const VNode* previous) {
		if (!node.control) return;

		ui::Window& control = *node.control;
		node.suppress_events = true;

		if (const VNode* text = first_text_child(next)) {
			node.descriptor->apply_text(control, text->text);
		}

		for (const auto& entry : next.props) {
			const std::wstring_view name = entry.first.view();
			if (name == L"key" || name == L"w-ref" || is_event_prop(name) || is_model_prop(name)) continue;
			if (is_layout_property(name)) continue;

			if (previous) {
				if (const PropertyValue* old = find_prop(*previous, name); old && *old == entry.second) continue;
			}

			if (!node.descriptor->apply_property(control, name, entry.second)) {
				debug_warn(L"unknown property \"" + std::wstring(name) + L"\" for this widget");
			}
		}

		if (const PropertyValue* ref = find_prop(next, L"w-ref")) {
			if (const auto* binding = ref->asRef(); binding && node.descriptor->set_ref) {
				node.ref_handle = binding->handle;
				if (!node.descriptor->set_ref(&control, binding->handle)) {
					debug_warn(L"w-ref type mismatch");
				}
			}
		}
		else if (node.ref_handle.valid() && node.descriptor->set_ref) {
			node.descriptor->set_ref(nullptr, node.ref_handle);
			node.ref_handle = RefHandle{};
		}

		node.suppress_events = false;
	}

	void Reconciler::rebuild_events(NodeState& node, const VNode& next) {
		if (!node.sink || !node.control) return;

		node.sink->clear();
		node.model_handle = RefHandle{};

		bool has_model = false;
		for (const auto& entry : next.props) {
			const std::wstring_view name = entry.first.view();

			if (is_event_prop(name)) {
				if (const auto* handler = entry.second.asHandler()) {
					EventHandler wrapped = [callback = *handler, &node](const EventPayload& payload) {
						if (node.suppress_events) return;
						callback(payload);
					};
					node.sink->on(std::wstring_view(name).substr(1), std::move(wrapped));
				}
				continue;
			}

			if (!is_model_prop(name)) continue;

			const auto* binding = entry.second.asRef();
			if (!binding) {
				debug_warn(L"w-model requires a Ref value");
				continue;
			}
			if (binding->type != std::type_index(typeid(std::wstring))) {
				throw std::invalid_argument("w32oop::ui: w-model:value requires Ref<std::wstring>");
			}

			node.model_handle = binding->handle;
			has_model = true;

			node.sink->on(L"update:value", [&node](const EventPayload& payload) {
				if (node.suppress_events) return;
				(void)node.model_handle.set_text(payload.text);
			});
		}

		if (!has_model) return;

		std::wstring current;
		if (node.model_handle.get_text(current) && node.control->text() != current) {
			node.suppress_events = true;
			node.control->text(current);
			node.suppress_events = false;
		}
	}

	void Reconciler::sync_layout(NodeState& node) const {
		if (!node.box) return;

		node.box->clear();
		for (const auto& entry : node.vnode.props) {
			if (!is_layout_property(entry.first.view())) continue;
			if (!apply_layout_property(*node.box, entry.first.view(), entry.second)) {
				debug_warn(L"bad value for layout property \"" + std::wstring(entry.first.view()) + L"\"");
			}
		}

		for (const auto& child : node.children) {
			if (child->control) {
				ui::FlexItem& item = node.box->add(*child->control);
				for (const auto& entry : child->vnode.props) {
					if (!is_layout_property(entry.first.view())) continue;
					if (!apply_layout_property(item, entry.first.view(), entry.second)) {
						debug_warn(L"bad value for layout property \"" + std::wstring(entry.first.view()) + L"\"");
					}
				}
			}
			else if (child->box) {
				ui::FlexItem& item = node.box->add(*child->box);
				for (const auto& entry : child->vnode.props) {
					if (!is_layout_property(entry.first.view())) continue;
					if (!apply_layout_property(item, entry.first.view(), entry.second)) {
						debug_warn(L"bad value for layout property \"" + std::wstring(entry.first.view()) + L"\"");
					}
				}
			}
			sync_layout(*child);
		}
	}

	void Reconciler::release_node(NodeState& node) const {
		if (node.ref_handle.valid() && node.descriptor && node.descriptor->set_ref) {
			node.descriptor->set_ref(nullptr, node.ref_handle);
			node.ref_handle = RefHandle{};
		}
		for (const auto& child : node.children) {
			if (child) release_node(*child);
		}
	}

	ui::Window* Reconciler::find(const std::wstring& key) const {
		if (!root_ || key.empty()) return nullptr;
		return find_in(*root_, key);
	}

	ui::Window* Reconciler::find_in(const NodeState& node, const std::wstring& key) const {
		if (node.key == key && node.control) return node.control.get();
		for (const auto& child : node.children) {
			if (!child) continue;
			if (ui::Window* found = find_in(*child, key)) return found;
		}
		return nullptr;
	}

	std::size_t Reconciler::mounted_count() const {
		std::size_t count = 0;
		if (root_) collect_count(*root_, count);
		return count;
	}

	void Reconciler::collect_count(const NodeState& node, std::size_t& count) const {
		if (node.control) ++count;
		for (const auto& child : node.children) {
			if (child) collect_count(*child, count);
		}
	}
}
