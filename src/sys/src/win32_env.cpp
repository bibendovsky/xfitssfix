// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <windows.h>
#include <shlobj.h>
#include <xfitssfix/encoding.h>
#include <xfitssfix/env.h>
#include <xfitssfix/exception.h>

namespace xfitssfix::env {

class EnvException : public Exception
{
public:
	explicit EnvException(const char* message)
		:
		Exception{"XFITSSFIX_SYS_ENV", message}
	{}
};

// --------------------------------------------------------------------------

fs::Path get_special_folder(SpecialFolderType special_folder_type)
{
	auto win32_csidl = -1;

	switch (special_folder_type)
	{
		case SpecialFolderType::app_data:
			win32_csidl = CSIDL_APPDATA;
			break;

		default:
			throw EnvException{"Unknown special folder type."};
	}

	WCHAR u16_buffer[MAX_PATH + 1];

	const auto win32_result = SHGetFolderPathW(
		nullptr,
		win32_csidl,
		nullptr,
		SHGFP_TYPE_CURRENT,
		u16_buffer);

	if (FAILED(win32_result))
	{
		throw EnvException{"Failed to get special folder."};
	}

	const auto u8_buffer = encoding::to_utf8(reinterpret_cast<const char16_t*>(u16_buffer));
	return fs::Path{u8_buffer.c_str(), static_cast<Int>(u8_buffer.size())};
}

// --------------------------------------------------------------------------

String get_var(const char* name)
{
	const auto u16_name = encoding::to_utf16(name);
	const auto u16_size_with_null = GetEnvironmentVariableW(reinterpret_cast<LPCWSTR>(u16_name.c_str()), nullptr, 0);

	if (u16_size_with_null == 0)
	{
		if (GetLastError() == ERROR_ENVVAR_NOT_FOUND)
		{
			throw EnvException{"Variable not found."};
		}

		return String{};
	}

	auto u16_value = U16String{};
	u16_value.resize(u16_size_with_null);

	const auto u16_size_without_null = GetEnvironmentVariableW(
		reinterpret_cast<LPCWSTR>(u16_name.c_str()),
		reinterpret_cast<LPWSTR>(u16_value.data()),
		u16_size_with_null);

	if (u16_size_without_null != (u16_size_with_null - 1))
	{
		throw EnvException{"Failed to get a variable."};
	}

	return encoding::to_utf8(u16_value.c_str());
}

} // namespace xfitssfix::env
