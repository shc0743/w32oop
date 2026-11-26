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


namespace w32oop::def {
	using w32FontHandle = w32BaseHandle<HFONT, false, DeleteObject, exceptions::invalid_font_handle_exception>;
}


namespace w32oop::ui {
	class Font final : public w32GUIObject {
	protected:
		w32FontHandle _font;
		struct Configuration {
			std::wstring name;
			float size = 0; // 逻辑大小。将基于缩放系数自动转换为物理大小
		} configuration;
		float _scale_factor = 1.0f;

	public:
		// 创建空的字体对象，直接使用它.get将会造成异常。
		Font() {}
		/// <summary>
		/// 创建字体对象
		/// </summary>
		/// <param name="font_name">字体名称</param>
		/// <param name="font_size">逻辑大小。将基于缩放系数自动转换为物理大小</param>
		Font(std::wstring font_name, float font_size = 14.0f) {
			configuration = Configuration{ .name = font_name, .size = font_size };
			if (validate_configuration()) update();
		}
		/// <summary>
		/// 创建字体对象
		/// </summary>
		/// <param name="config">配置</param>
		Font(Configuration config) : configuration(config) {
			if (validate_configuration()) update();
		}

		// create new handle when copy.
		Font(const Font& other)
			: configuration(other.configuration),
			_scale_factor(other._scale_factor) {
			if (validate_configuration()) update();
		}
		Font& operator=(const Font& other) {
			if (this != &other) {
				configuration = other.configuration;
				_scale_factor = other._scale_factor;
				if (validate_configuration()) update();
			}
			return *this;
		}
		// directly move.
		Font(Font&&) = default;
		Font& operator=(Font&&) = default;

	public:
		// 检查配置是否有效。
		bool validate_configuration() const noexcept;
		// 该函数隐式创建字体对象，以检测对象是否有效(因此它不是 const)。如果不想要隐式创建对象的行为，请使用 now_valid()
		bool is_valid() {
			if (_font.is_valid()) return true;
			// 对于配置不正确的情况，快速返回失败，避免额外的异常开销
			if (!validate_configuration()) return false;
			// 仍然需要实际创建以检查到底是否有效（即使配置正确也可能实际上无效，例如字体名称不存在。）
			try {
				return get();
			}
			catch (...) { return false; }
		}
		bool now_valid() const noexcept { return _font.is_valid(); }
		/// <summary>
		/// 更新字体对象。更新后原有的字体将失效，使用它的任何对象都需要重新设置字体。
		/// </summary>
		/// <param name="forceCreate">是否强制创建。默认情况下，若不强制创建，则函数在没有现有字体的时候不会创建它，以优化性能。</param>
		void update(bool forceCreate = false);
		// 显式创建字体
		inline void create(bool failIfCreated = true) {
			if (failIfCreated) if (now_valid()) throw exceptions::font_already_created_exception();
			update(true);
		}

		inline std::wstring name() const { return configuration.name; }
		inline void name(std::wstring new_name) {
			if (configuration.name != new_name) {
				configuration.name = new_name;
				if (validate_configuration()) update();
			}
		}

		inline float size() const { return configuration.size; }
		inline void size(float new_size) {
			if (configuration.size != new_size) {
				configuration.size = new_size;
				if (validate_configuration()) update();
			}
		}

		inline float scale_factor() const {
			return _scale_factor;
		}

		// 更新该字体的缩放系数。在对应的窗口发生DPI更改后应调用此函数，然后重新设置窗口的字体为get()的值。
		inline void scale_factor(float value) {
			if (value != _scale_factor) {
				_scale_factor = value;
				if (validate_configuration()) update();
			}
		}

	public:
		// 获取字体（若字体没有创建会自动创建它）
		HFONT get() {
			if (!now_valid()) {
				create(false);
			}
			return _font;
		}
		// 直接获取原始字体句柄值（可能返回null）
		HFONT get_value() const noexcept {
			return _font.get();
		}

	};
}


