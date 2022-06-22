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

} // namespace xfitssfix::env
