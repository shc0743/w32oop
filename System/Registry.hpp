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
#include <any>


namespace w32oop::exceptions {
	w32oop_declare_exception_class_from(registry, system_exception);
	w32oop_declare_exception_class_from(invalid_registry_handle, registry_exception);
	w32oop_declare_exception_class_from(registry_value_type_not_supported, registry_exception);
	w32oop_declare_exception_class_from(registry_query_failed, registry_exception);
	w32oop_declare_exception_class_from(registry_write_failed, registry_exception);
	w32oop_declare_exception_class_from(registry_key_exists, registry_exception);
	w32oop_declare_exception_class_from(registry_key_not_exist, registry_exception);
	w32oop_declare_exception_class_from(registry_enum_failed, registry_exception);
}


namespace w32oop::system {
	bool WINAPI w32RegCloseKey(HKEY hKey);
}


namespace w32oop::def {
	using w32RegistryHandle = w32BaseHandle<HKEY, false, system::w32RegCloseKey, exceptions::invalid_registry_handle_exception>;


	class w32RegistryObject : public w32SystemObject {
	public:
		w32RegistryObject() = default;
		~w32RegistryObject() = default;
	};
}


namespace w32oop::system {
	class RegistryKey;
	class RegistryValue : public w32RegistryObject {
	protected:
		std::any m_value;
		ULONG m_type = REG_NONE; // 默认类型为无效类型
	public:
		RegistryValue() {}
		explicit RegistryValue(std::any&& value, ULONG type) : m_value(std::move(value)), m_type(type) {}
		RegistryValue(const RegistryValue&) = default; // 允许拷贝构造
		RegistryValue& operator=(const RegistryValue&) = default; // 允许拷贝赋值
		RegistryValue(RegistryValue&&) noexcept = default; // 允许移动构造
		RegistryValue& operator=(RegistryValue&&) noexcept = default; // 允许移动赋值
	public:
		template<typename value_type>
		value_type get(ULONG type = 0) const {
			if (type != 0 && m_type != type) {
				throw exceptions::invalid_registry_handle_exception("Invalid registry value type.");
			}
			try {
				return std::any_cast<value_type>(m_value);
			}
			catch (const std::bad_any_cast&) {
				throw exceptions::invalid_registry_handle_exception("Invalid type cast for registry value.");
			}
		}
		inline ULONG type() const {
			return m_type;
		}
	};
	class RegistryKey final : public w32RegistryObject {
	protected:
		w32RegistryHandle hKey;

	public:
		RegistryKey(HKEY key = HKEY_LOCAL_MACHINE) : hKey(key) {}
		RegistryKey(HKEY key, wstring path, REGSAM access = KEY_ALL_ACCESS) {
			HKEY _ = 0;
			(void)RegOpenKeyExW(key, path.c_str(), 0, access, &_);
			hKey = _;
			hKey.validate();
		}
		RegistryKey(const RegistryKey& key, wstring path, REGSAM access = KEY_ALL_ACCESS) {
			HKEY _ = 0;
			(void)RegOpenKeyExW(key.hKey, path.c_str(), 0, access, &_);
			hKey = _;
			hKey.validate();
		}
		RegistryKey(const RegistryKey&) = delete; // 禁止拷贝构造
		RegistryKey& operator=(const RegistryKey&) = delete; // 禁止拷贝赋值
		RegistryKey(RegistryKey&& other) noexcept : hKey(std::move(other.hKey)) {}
		RegistryKey& operator=(RegistryKey&& other) noexcept {
			if (this != &other) {
				hKey = std::move(other.hKey);
			}
			return *this;
		}
		~RegistryKey() {}

	public:
		// 创建子健
		RegistryKey create(wstring subkeyName, REGSAM access = KEY_ALL_ACCESS, bool bFailIfExists = false);
		// 打开子健
		RegistryKey open(wstring subkeyName, REGSAM access = KEY_ALL_ACCESS);
		// 获取特定键的值，valueName可以为空字符串，表示获取(默认)键的值
		RegistryValue get(wstring valueName, bool bNoExpand = false) const;
		// 设置特定键的值
		void set(wstring valueName, const RegistryValue& value);
		// 设置(默认)键的值
		inline void set(const RegistryValue& value) {
			set(L"", value);
		}
		bool delete_key(wstring subkeyName, REGSAM samDesired = KEY_WOW64_64KEY);
		bool delete_value(wstring valueName);
		bool delete_key_value(wstring subkeyName, wstring valueName);
		bool exists(wstring subkeyName) const;
		bool exists_value(wstring valueName) const;
		template<typename T = std::vector<std::wstring>>
		T keys() const {
			T keyNames{};
			DWORD index = 0;
			constexpr size_t maxKeyNameLength = 2048; // 最大键名长度
			wchar_t keyName[maxKeyNameLength]{};
			DWORD keyNameLength = (DWORD)maxKeyNameLength;

			while (true) {
				keyNameLength = maxKeyNameLength;
				LSTATUS result = RegEnumKeyExW(
					hKey, index, keyName, &keyNameLength,
					0, 0, 0, 0
				);

				if (result == ERROR_SUCCESS) {
					keyNames.push_back(std::wstring(keyName, keyNameLength));
					index++;
				}
				else if (result == ERROR_NO_MORE_ITEMS) {
					break;
				}
				else throw exceptions::registry_enum_failed_exception("Failed to enumerate registry keys");
			}
			return keyNames;
		}
		template<typename T = std::vector<std::pair<std::wstring, RegistryValue>>>
		T values() const {
			// Windows 默认机制实在太过复杂，我们间接使用
			auto keys = this->keys();
			// 然后获取每个键的值
			T values{};
			for (auto& i : keys) {
				values.push_back(std::make_pair(i, get(i)));
			}
			return values;
		}
	};
}

