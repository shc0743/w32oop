#include "./Menu.hpp"
#include "./Window.hpp"
#include "../Utility/RAII.hpp"
using namespace w32oop;
using namespace w32oop::ui;

w32MenuHandleEx w32oop::ui::Menu::build(HMENU(WINAPI* builder)(VOID)) const {
	w32MenuHandleEx hMenu(builder());
	std::optional<UINT> default_menu_pos;
	UINT pos = 0;
	for (auto& i : children) {
		if (!_build_itermenu(hMenu, i, hMenu)) continue; // 构建每个菜单项
		if (i.is_default()) {
			if (default_menu_pos.has_value()) throw w32oop::exceptions::multiple_default_menu_item_exception(
				format("Duplicate default menu item found at position {}", pos));
			default_menu_pos = pos;
		}
		++pos;
	}
	if (default_menu_pos.has_value()) SetMenuDefaultItem(hMenu, default_menu_pos.value(), TRUE);
	return hMenu;
}
bool w32oop::ui::Menu::_build_itermenu(HMENU hMenu, const MenuItem& item, w32MenuHandleEx& owner) {
	// 菜单的合法类型只有MF_POPUP,MF_SEPARATOR,MF_STRING
	if (item.is_container()) {
		HMENU subMenu(CreatePopupMenu());
		AppendMenuW(hMenu, item.type(), reinterpret_cast<UINT_PTR>(subMenu), item.text().c_str());
		_build_menutree(subMenu, item, owner);
		//顶层菜单的销毁会导致所有子菜单被系统销毁，因此我们不应该再通过RAII来销毁子菜单
		return true;
	}

	// 构造菜单内容
	if (item.icon() && item.is_string()) {
		MENUITEMINFO mii{};
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_ID;
		mii.dwTypeData = const_cast<PWSTR>(item.text().c_str());
		mii.wID = item.id();
		ICONINFO iconInfo{};
		if (!GetIconInfo(item.icon(), &iconInfo)) {
			// GetIconInfo 失败时退化为纯文本菜单项
			AppendMenuW(hMenu, item.type(), item.id(), item.text().c_str());
			return true;
		}
		mii.hbmpItem = iconInfo.hbmColor;
		owner.bmps.insert(mii.hbmpItem); // 将位图句柄存储到菜单句柄中，以便 RAII 自动释放
		owner.bmps.insert(iconInfo.hbmMask); // hbmMask 同样由 GetIconInfo 新建，也需要释放
		InsertMenuItemW(hMenu, item.id(), FALSE, &mii);
		return true;
	}
	if (item.is_string()) {
		UINT extraFlags = (item.checked() ? MF_CHECKED : 0);
		AppendMenuW(hMenu, item.type() | extraFlags, item.id(), item.text().c_str());
		return true;
	}
	if (item.is_separator()) {
		AppendMenuW(hMenu, item.type(), 0, 0);
		return true;
	}
	return false;
}
void w32oop::ui::Menu::_build_menutree(HMENU hMenu, const MenuItem& item, w32MenuHandleEx& owner) {
	for (auto& i : item.get_children()) {
		_build_itermenu(hMenu, i, owner); // 构建每个子菜单项
	}
}

int w32oop::ui::Menu::pop(long x, long y, bool run_handler, Window* owner) {
	w32MenuHandleEx hMenu = build(); // 构建菜单句柄
	if (owner && owner->is_framework_dpi_virtualization_allowed()) {
		// 虚拟化坐标
		x = owner->scaled(x);
		y = owner->scaled(y);
	}
	HWND hostWindow = owner ? *owner : CreateWindowExW(WS_EX_TOOLWINDOW, L"Static", L"Popup Menu Window",
		WS_POPUP, -8, -8, 1, 1, NULL, 0, NULL, NULL);
	if (!hostWindow) {
		throw exceptions::window_creation_failure_exception("Failed to create temporary window for menu popup.");
	}
	if (!owner) ShowWindow(hostWindow, SW_SHOW);
	SetForegroundWindow(hostWindow);
	int result = TrackPopupMenu(
		hMenu, // 菜单句柄
		TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
		x, y, 0,
		hostWindow, NULL
	);
	if (!owner) DestroyWindow(hostWindow);
	// 处理用户选择
	if (run_handler) {
		if (result != 0) run(result);
	}
	return result;
}

bool w32oop::ui::Menu::run(int nId) {
	// 找到对应的菜单项并运行
	try {
		for (auto& i : children) {
			if (i.id() == nId) {
				i.click();
				return true;
			}
			if (i.is_container()) if (_run_handler_for_item(i, nId)) return true;
		}
	}
	catch (const exception& exc) {
		if (exc_handler) {
			if (!exc_handler(exc)) throw;
		}
		else throw; // 如果没有处理器，重新抛出异常
	}
	return false;
}

bool w32oop::ui::Menu::_run_handler_for_item(const MenuItem& item, int nId) {
	for (auto& i : item.get_children()) {
		if (i.id() == nId) {
			i.click();
			return true;
		}
		if (i.is_container()) {
			if (_run_handler_for_item(i, nId)) return true;
		}
	}
	return false;
}

void w32oop::ui::MenuItem::click() const {
	if (onClickHandler) {
		onClickHandler();
	}
	// TODO: 判断是否需要切换checked状态…
}
