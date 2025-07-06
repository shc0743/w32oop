#include "Registry.hpp"
using namespace w32oop;
using namespace w32oop::system;


bool WINAPI w32oop::system::w32RegCloseKey(HKEY hKey) {
	// 先判断是不是那些系统预定义的键
	if (hKey == HKEY_CLASSES_ROOT || hKey == HKEY_CURRENT_USER || hKey == HKEY_LOCAL_MACHINE ||
		hKey == HKEY_USERS || hKey == HKEY_PERFORMANCE_DATA || hKey == HKEY_CURRENT_CONFIG ||
		hKey == HKEY_DYN_DATA) {
		return false; // 不能关闭这些预定义的键
	}
	return ::RegCloseKey(hKey) == ERROR_SUCCESS;
}

RegistryKey w32oop::system::RegistryKey::create(wstring subkeyName, REGSAM access, bool bFailIfExists) {
	HKEY hSubKey = 0; DWORD dwDisposition = 0;
	auto result = RegCreateKeyExW(hKey, subkeyName.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE,
		access, nullptr, &hSubKey, &dwDisposition);
	if (result == ERROR_SUCCESS) {
		if (bFailIfExists && dwDisposition == REG_OPENED_EXISTING_KEY) {
			RegCloseKey(hSubKey);
			throw exceptions::registry_key_exists_exception("Registry key already exists.");
		}
		return RegistryKey(hSubKey);
	}
	else {
		throw exceptions::invalid_registry_handle_exception("Failed to create registry key.");
	}
}

RegistryKey w32oop::system::RegistryKey::open(wstring subkeyName, REGSAM access) {
	HKEY hSubKey = 0;
	auto result = RegOpenKeyExW(hKey, subkeyName.c_str(), 0, access, &hSubKey);
	if (result == ERROR_SUCCESS) {
		return RegistryKey(hSubKey);
	}
	else {
		throw exceptions::invalid_registry_handle_exception("Failed to open registry key.");
	}
}

RegistryValue w32oop::system::RegistryKey::get(wstring valueName, bool bNoExpand) const {
	std::any val;
	DWORD type = 0, size = 0;
	// 查询值的大小
	auto result = RegQueryValueExW(hKey, valueName.c_str(), nullptr, &type, nullptr, &size);
	if (result != ERROR_SUCCESS) {
		throw exceptions::registry_query_failed_exception("Failed to query registry value size");
	}
	if (size == 0) {
		return RegistryValue(); // 如果没有值，返回空的 RegistryValue
	}
	auto buffer = std::make_unique<BYTE[]>(static_cast<size_t>(size) + 1);
	switch (type) {
	case REG_SZ:
	case REG_EXPAND_SZ:
	case REG_MULTI_SZ:
	{
		DWORD flag = (type == REG_SZ) ? (RRF_RT_REG_SZ) :
			((type == REG_EXPAND_SZ) ? RRF_RT_REG_EXPAND_SZ : RRF_RT_REG_MULTI_SZ);
		if (bNoExpand) flag |= RRF_NOEXPAND;

		// （以字节为单位）
		DWORD mySize = size, myType = type;
		auto result = RegGetValueW(hKey, nullptr, valueName.c_str(), flag, &myType, buffer.get(), &mySize);
		if (result != ERROR_SUCCESS) {
			if (result == ERROR_MORE_DATA) {
				// Unexpected!
			}
			if (result == ERROR_FILE_NOT_FOUND) {
				throw exceptions::registry_key_not_exist_exception("Registry key does not exist.");
			}
			throw exceptions::registry_query_failed_exception("Failed to get registry value.");
		}
		if (myType != type) throw exceptions::registry_query_failed_exception("Registry data changed during program execution.");

		if (type == REG_MULTI_SZ) {
			// 处理多字符串类型
			const wchar_t* p = reinterpret_cast<wchar_t*>(buffer.get());
			const wchar_t* end = p + (mySize / sizeof(wchar_t));
			vector<wstring> values;

			while (p < end && *p != L'\0') {
				const wchar_t* start = p;
				// 找到当前字符串的结尾
				while (p < end && *p != L'\0') {
					++p;
				}
				values.emplace_back(start, p);
				++p; // 跳过null字符
			}

			val = values;
			return RegistryValue(move(val), type);
		}

		// mySize 将包含终止null
		wstring value(reinterpret_cast<wchar_t*>(buffer.get()), (size_t(mySize) - 2) / sizeof(wchar_t));
		val = value;
		return RegistryValue(move(val), type);
	}
		break;
	case REG_DWORD:
	case REG_QWORD:
	case REG_DWORD_BIG_ENDIAN:
	case REG_BINARY: 
	{
		// 直接获取数据
		DWORD mySize = size, myType = type;
		auto result = RegQueryValueExW(hKey, valueName.c_str(), nullptr, &myType, buffer.get(), &mySize);
		if (result != ERROR_SUCCESS) {
			if (result == ERROR_FILE_NOT_FOUND) {
				throw exceptions::registry_key_not_exist_exception("Registry key does not exist.");
			}
			throw exceptions::registry_query_failed_exception("Failed to query registry value.");
		}
		if (myType != type) throw exceptions::registry_query_failed_exception("Registry data changed during program execution.");
		if (type == REG_DWORD) {
			DWORD value = *reinterpret_cast<DWORD*>(buffer.get());
			val = value;
		}
		else if (type == REG_QWORD) {
			ULONGLONG value = *reinterpret_cast<ULONGLONG*>(buffer.get());
			val = value;
		}
		else if (type == REG_DWORD_BIG_ENDIAN) {
			DWORD value = _byteswap_ulong(*reinterpret_cast<DWORD*>(buffer.get()));
			val = value;
		}
		else if (type == REG_BINARY) {
			vector<BYTE> value(buffer.get(), buffer.get() + mySize);
			val = value;
		}
		return RegistryValue(move(val), type);
	}
		break;
	default:
		// currently REG_LINK and REG_NONE are not supported
		throw exceptions::registry_value_type_not_supported_exception("Registry value type not supported.");
	}
}

void w32oop::system::RegistryKey::set(wstring valueName, const RegistryValue& value) {
	DWORD type = value.type();
	if (type == REG_NONE) {
		throw exceptions::registry_value_type_not_supported_exception("Cannot set a value of type REG_NONE.");
	}
	DWORD size = 0;
	if (type == REG_SZ || type == REG_EXPAND_SZ) {
		const auto& strValue = value.get<wstring>();
		size = static_cast<DWORD>((strValue.size() + 1) * sizeof(wchar_t)); // 包括终止null
		PCWSTR cstr = strValue.c_str();
		auto result = RegSetValueExW(hKey, valueName.c_str(), 0, type, reinterpret_cast<const BYTE*>(cstr), size);
		if (result != ERROR_SUCCESS) {
			throw exceptions::registry_write_failed_exception("Failed to set registry value.");
		}
	}
	else if (type == REG_MULTI_SZ) {
		const auto& multiStrValue = value.get<vector<wstring>>();
		size = 0;
		for (const auto& str : multiStrValue) {
			size += static_cast<DWORD>((str.size() + 1) * sizeof(wchar_t)); // 每个字符串后面都有一个终止null
		}
		size += sizeof(wchar_t); // 最后一个终止null
		auto buffer = std::make_unique<char[]>(size); // 字节数组
		memset(buffer.get(), 0, size); // 初始化为0
		// 填充多字符串（使用内存复制 API）
		size_t pointer = 0;
		for (const auto& str : multiStrValue) {
			size_t len = str.size();
			memcpy(buffer.get() + pointer, str.c_str(), (len + 1) * sizeof(wchar_t)); // 包括终止null
			pointer += (len + 1) * sizeof(wchar_t);
		}
		auto result = RegSetValueExW(hKey, valueName.c_str(), 0, type, reinterpret_cast<const BYTE*>(buffer.get()), size);
		if (result != ERROR_SUCCESS) {
			throw exceptions::registry_write_failed_exception("Failed to set registry value.");
		}
	}
	else if (type == REG_QWORD) {
		size = sizeof(ULONGLONG);
		auto data = value.get<ULONGLONG>();
		auto result = RegSetValueExW(hKey, valueName.c_str(), 0, type, reinterpret_cast<const BYTE*>(&data), size);
		if (result != ERROR_SUCCESS) {
			throw exceptions::registry_write_failed_exception("Failed to set registry value.");
		}
	}
	else if (type == REG_DWORD || type == REG_DWORD_BIG_ENDIAN) {
		size = sizeof(DWORD);
		auto data = value.get<DWORD>();
		if (type == REG_DWORD_BIG_ENDIAN) {
			data = _byteswap_ulong(data); // 转换为大端字节序
		}
		auto result = RegSetValueExW(hKey, valueName.c_str(), 0, type, reinterpret_cast<const BYTE*>(&data), size);
		if (result != ERROR_SUCCESS) {
			throw exceptions::registry_write_failed_exception("Failed to set registry value.");
		}
	}
	else if (type == REG_BINARY) {
		const auto& binValue = value.get<vector<BYTE>>();
		size = static_cast<DWORD>(binValue.size());
		auto result = RegSetValueExW(hKey, valueName.c_str(), 0, type, binValue.data(), size);
		if (result != ERROR_SUCCESS) {
			throw exceptions::registry_write_failed_exception("Failed to set registry value.");
		}
	}
	else {
		throw exceptions::registry_value_type_not_supported_exception("Registry value type not supported.");
	}
}

bool w32oop::system::RegistryKey::delete_key(wstring subkeyName, REGSAM samDesired) {
	auto r = RegDeleteKeyExW(hKey, subkeyName.c_str(), samDesired, 0);
	if (r == ERROR_SUCCESS) return true;
	SetLastError(r);
	return false;
}

bool w32oop::system::RegistryKey::delete_value(wstring valueName) {
	auto r = RegDeleteValueW(hKey, valueName.c_str());
	if (r == ERROR_SUCCESS) return true;
	SetLastError(r);
	return false;
}

bool w32oop::system::RegistryKey::delete_key_value(wstring subkeyName, wstring valueName) {
	auto r = RegDeleteKeyValueW(hKey, subkeyName.c_str(), valueName.c_str());
	if (r == ERROR_SUCCESS) return true;
	SetLastError(r);
	return false;
}

bool w32oop::system::RegistryKey::exists(wstring subkeyName) const {
	HKEY hSubKey = 0;
	auto result = RegOpenKeyExW(hKey, subkeyName.c_str(), 0, KEY_READ, &hSubKey);
	if (result == ERROR_SUCCESS) {
		RegCloseKey(hSubKey);
		return true;
	}
	return false;
}

bool w32oop::system::RegistryKey::exists_value(wstring valueName) const {
	DWORD type;
	auto result = RegQueryValueExW(hKey, valueName.c_str(), nullptr, &type, nullptr, nullptr);
	return result == ERROR_SUCCESS;
}
