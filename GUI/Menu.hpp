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
#include <set>


namespace w32oop::exceptions {
	w32oop_declare_exception_class_from(invalid_menu_handle, ui_exception);
	w32oop_declare_exception_class_from(invalid_menu_state, ui_exception);
}


namespace w32oop::def {
	// DestroyMenu 是递归的，也就是说，它将销毁菜单及其所有子菜单。
	using w32MenuHandle = w32BaseHandle<HMENU, false, DestroyMenu, exceptions::invalid_menu_handle_exception>;
	class w32MenuHandleEx : public w32MenuHandle {
	public:
		w32MenuHandleEx() : w32MenuHandle() {}
		w32MenuHandleEx(HMENU hMenu) : w32MenuHandle(hMenu) {}
		w32MenuHandleEx(const w32MenuHandleEx& other) = delete;
		w32MenuHandleEx(w32MenuHandleEx&& other) noexcept = default;
		w32MenuHandleEx& operator=(const w32MenuHandle& other) = delete;
		w32MenuHandleEx& operator=(w32MenuHandleEx&& other) noexcept {
			if (this != &other) {
				w32MenuHandle::operator=(std::move(other)); // 调用基类的移动赋值操作
				bmps = std::move(other.bmps); // 移动位图句柄集合
				other.bmps.clear(); // 清空其他对象的位图句柄集合
			}
			return *this; // 返回自身以支持链式调用
		}

		virtual ~w32MenuHandleEx() {
			// 析构时，自动销毁所有子菜单和位图
			for (auto& bmp : bmps) {
				if (bmp) {
					DeleteObject(bmp); // 删除位图对象
				}
			}
		}

	public:
		set<HBITMAP> bmps; // 存储位图句柄
	};
}


namespace w32oop::ui {
	class MenuItem : public w32MenuObject {
	protected:
		UINT m_type;
		UINT m_id; // 注：这里的 ID 字段仅仅要求唯一就行了，只起识别作用，不需要手动处理
		wstring m_text;
		bool is_checked = false;
		HICON hIcon = NULL;
		vector<MenuItem> children;

	public:
		MenuItem(UINT type = MF_STRING, UINT id = 0) : m_type(type), m_id(id) {}
		MenuItem(const wstring& text, UINT id) : m_text(text), m_type(MF_STRING), m_id(id) {}
		MenuItem(const wstring& text, UINT id, function<void()> onClickHandler) :
			m_text(text), m_type(MF_STRING), m_id(id), onClickHandler(onClickHandler) {}
		MenuItem(const wstring& text, vector<MenuItem> children) :
			m_text(text), m_type(MF_STRING | MF_POPUP), m_id(0), children(children) {}
		static MenuItem separator() {
			return MenuItem(MF_SEPARATOR); // 创建分隔符菜单项
		}
		static MenuItem container() {
			return MenuItem(MF_POPUP); // 创建容器菜单项
		}
		explicit MenuItem(vector<MenuItem> items) {
			m_type = MF_POPUP; // 设置为容器类型
			m_id = 0; // 容器菜单项没有ID
			children = std::move(items); // 移动子菜单项
		}

		bool operator==(const MenuItem& other) const {
			if (this->m_type != other.m_type) return false;
			if (this == &other) return true;
			return false;
		}

	public:
		const UINT& id() const {
			return m_id;
		}
		const UINT& type() const {
			return m_type;
		}
		const wstring& text() const {
			return m_text; // 返回菜单项文本
		}
		void text(const wstring& newText) {
			m_text = newText; // 设置菜单项文本
		}
		HICON icon() const {
			return hIcon; // 返回菜单项图标
		}
		void icon(HICON newIcon) {
			hIcon = newIcon; // 设置菜单项图标
		}
		bool checked() const {
			return is_checked; // 返回是否选中
		}
		inline void check(bool bChecked = true) {
			is_checked = bChecked; // 设置选中状态
		}
		inline void uncheck() {
			check(false); // 取消选中
		}

	protected:
		function<void()> onClickHandler; // 点击事件处理器
	public:
		function<void()> onClick() const {
			return onClickHandler; // 获取点击事件处理器
		}
		void onClick(function<void()> handler) {
			onClickHandler = handler; // 设置点击事件处理器
		}

	public:
		// 子菜单相关功能
		const vector<MenuItem>& get_children() const {
			return children;
		}
		vector<MenuItem>& get_children() {
			return children;
		}
		inline bool is_container() const {
			return (m_type & MF_POPUP) != 0; // 判断是否为容器菜单项
		}
		MenuItem& append(MenuItem item) {
			if (!is_container()) {
				throw exceptions::invalid_menu_state_exception("Cannot append to a non-container MenuItem");
			}
			children.push_back(std::move(item)); // 添加子菜单项
			return *this; // 返回自身以支持链式调用
		}
		MenuItem& append(const wstring& text, UINT id) {
			return append(MenuItem(text, id)); // 添加文本菜单项
		}
		// 移除
		void clear() {
			if (!is_container()) {
				throw exceptions::invalid_menu_state_exception("Cannot clear a non-container MenuItem");
			}
			children.clear(); // 清空子菜单项
		}
		void remove(const MenuItem& item) {
			if (!is_container()) {
				throw exceptions::invalid_menu_state_exception("Cannot remove from a non-container MenuItem");
			}
			children.erase(std::remove(children.begin(), children.end(), item), children.end()); // 移除指定子菜单项
		}
		void remove(size_t index) {
			if (!is_container()) {
				throw exceptions::invalid_menu_state_exception("Cannot remove from a non-container MenuItem");
			}
			if (index < children.size()) {
				children.erase(children.begin() + index); // 移除指定索引的子菜单项
			}
		}

	public:
		void click() const; // 点击/选择
	};
	class Menu : public w32MenuObject {
	protected:
		vector<MenuItem> children;

	public:
		Menu() {}
		Menu(MenuItem item) : children{ std::move(item) } {} // 使用单个 MenuItem 构造菜单
		Menu(vector<MenuItem> items) : children(std::move(items)) {} // 使用移动语义构造菜单
#if 0
		// 从资源中构建菜单。动态解析资源文件中的菜单结构并转换为 MenuItem 对象。
		static Menu build_from_resource(HMENU hMenu);
		// TODO: 未实现
#endif

	public:
		// 核心实现，动态构造菜单
		w32MenuHandleEx build(HMENU(WINAPI* builder)(VOID) = CreatePopupMenu) const;
		// 弹出菜单（显示菜单）。
		int pop(long x, long y, bool run_handler = true);
		// 在鼠标所在位置弹出菜单（显示菜单）。
		inline int pop() {
			POINT pt{};
			GetCursorPos(&pt); // 获取鼠标位置
			return pop(pt); // 弹出菜单
		}
		// 在指定位置弹出菜单（显示菜单）。
		inline int pop(const POINT& pt) {
			return pop(pt.x, pt.y); // 弹出菜单
		}
		// 运行对应的处理程序
		bool run(int nId);
	private:
		bool _run_handler_for_item(const MenuItem& item, int nId);
		static void _build_itermenu(HMENU hMenu, const MenuItem& item, w32MenuHandleEx& owner);
		static void _build_menutree(HMENU hMenu, const MenuItem& item, w32MenuHandleEx& owner);

	public:
		vector<MenuItem>& get_children() {
			return children; // 获取子菜单项
		}
		const vector<MenuItem>& get_children() const {
			return children; // 获取子菜单项
		}
		
	protected:
		std::function<bool(const exception&)> exc_handler;
	public:
		// 异常处理器返回true表示异常已被处理，返回false则重新向上抛
		void set_exception_handler(std::function<bool(const exception&)> handler) {
			exc_handler = handler; // 设置异常处理器
		}
	};
}
