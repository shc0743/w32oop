#include "./Menu.hpp"
using namespace w32oop;
using namespace w32oop::ui;

w32MenuHandleEx w32oop::ui::Menu::build(HMENU(WINAPI* builder)(VOID)) const {
	w32MenuHandleEx hMenu(builder());
	for (auto& i : children) {
		_build_itermenu(hMenu, i, hMenu); // 构建每个菜单项
	}
	return hMenu;
}
void w32oop::ui::Menu::_build_itermenu(HMENU hMenu, const MenuItem& item, w32MenuHandleEx& owner) {
	if (item.is_container()) {
		HMENU subMenu(CreatePopupMenu());
		AppendMenuW(hMenu, item.type(), reinterpret_cast<UINT_PTR>(subMenu), item.text().c_str());
		_build_menutree(subMenu, item, owner);
		//顶层菜单的销毁会导致所有子菜单被系统销毁，因此我们不应该再通过RAII来销毁子菜单
		return;
	}

	// 构造菜单内容
	if (item.icon() && (item.type() == MF_STRING)) {
		MENUITEMINFO mii{};
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_ID;
		mii.dwTypeData = const_cast<PWSTR>(item.text().c_str());
		mii.wID = item.id();
		ICONINFO iconInfo;
		GetIconInfo(item.icon(), &iconInfo);
		mii.hbmpItem = iconInfo.hbmColor;
		owner.bmps.insert(mii.hbmpItem); // 将位图句柄存储到菜单句柄中，以便 RAII 自动释放
		InsertMenuItemW(hMenu, item.id(), FALSE, &mii);
		return;
	}
	if (item.type() == MF_STRING) {
		AppendMenuW(hMenu, item.type(), item.id(), item.text().c_str());
	}
	if (item.type() == MF_SEPARATOR) {
		AppendMenuW(hMenu, item.type(), 0, 0);
	}
}
void w32oop::ui::Menu::_build_menutree(HMENU hMenu, const MenuItem& item, w32MenuHandleEx& owner) {
	for (auto& i : item.get_children()) {
		_build_itermenu(hMenu, i, owner); // 构建每个子菜单项
	}
}

int w32oop::ui::Menu::pop(long x, long y, bool run_handler) {
	w32MenuHandleEx hMenu = build(); // 构建菜单句柄
	HWND tempWindow = CreateWindowExW(WS_EX_TOOLWINDOW, L"Static", L"Popup Menu Window",
		WS_POPUP, 0, 0, 1, 1, NULL, 0, NULL, NULL);
	if (!tempWindow) {
		throw exceptions::window_creation_failure_exception("Failed to create temporary window for menu popup.");
	}
	ShowWindow(tempWindow, SW_NORMAL);
	SetForegroundWindow(tempWindow);
	int result = TrackPopupMenu(
		hMenu, // 菜单句柄
		TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
		x, y, 0,
		tempWindow, NULL
	);
	// 处理用户选择
	if (run_handler) {
		if (result != 0) run(result);
	}
	DestroyWindow(tempWindow); // 销毁临时窗口
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
