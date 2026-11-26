#include "./Font.hpp"

bool w32oop::ui::Font::validate_configuration() const noexcept {
	if (!configuration.size || configuration.name.empty()) return false;
	return true;
}

void w32oop::ui::Font::update(bool forceCreate) {
	if (!forceCreate && !is_valid()) {
		// did not create, and no forceCreate present, skip create
		return;
	}
	if (!validate_configuration()) throw exceptions::invalid_font_exception();
	_font = CreateFontW(
		(int)(-configuration.size * _scale_factor - 0.5f), (int)(-(configuration.size / 2) * _scale_factor - 0.5f),
		0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE,
		configuration.name.c_str());
	if (!_font.is_valid()) throw exceptions::font_creation_failure_exception();
}
