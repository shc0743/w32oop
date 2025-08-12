#include "FoundationLib.hpp"
using namespace w32oop;
std::atomic<unsigned long long> w32oop::ui::BaseSystemWindow::ctlid_generator;




bool w32oop::ui::BaseSystemWindow::class_registered() const {
	return true;
}


HWND w32oop::ui::BaseSystemWindow::new_window() {
	auto cls = get_class_name();
	return CreateWindowExW(
		setup_info->styleEx,
		cls.c_str(),
		setup_info->title.c_str(),
		setup_info->style,
		setup_info->x, setup_info->y,
		setup_info->width, setup_info->height,
		parent_window, // 必须提供，否则会失败（逆天Windows控件库。。。）并且不可以变化，否则丢消息。。。
		(HMENU)(LONG_PTR)(ctlid), GetModuleHandle(NULL), nullptr
	);
}

wstring w32oop::ui::foundation::StatusBar::get_text(int part) const {
	int len = LOWORD(send(SB_GETTEXTLENGTH, part, 0));
	if (!len || len < 0 || len > 32768) return wstring();
	auto buffer = std::make_unique<WCHAR[]>(static_cast<size_t>(len) + 1);
	send(SB_GETTEXTW, part, (LPARAM)buffer.get());
	return buffer.get();
}
