// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <xfitssfix/platform.h>
#include <cassert>
#include <xfitssfix/core_types.h>
#include <xfitssfix/exception.h>
#include <xfitssfix/fs.h>

#ifdef XFITSSFIX_WIN32
#include <windows.h>
#include <memory>
#include <xfitssfix/encoding.h>
#endif // XFITSSFIX_WIN32

namespace xfitssfix::fs {

namespace {

struct ErrorMessages
{
	static constexpr auto get_working_dir = "Failed to get working directory.";
	static constexpr auto set_working_dir = "Failed to set working directory.";
	static constexpr auto get_absolute_path = "Failed to get absolute path.";
	static constexpr auto create_dir = "Failed to create directory.";
	static constexpr auto create_dirs = "Failed to create directory recursively.";
	static constexpr auto rename = "Failed to rename.";
	static constexpr auto remove = "Failed to remove.";
};

class FileSystemException : public Exception
{
public:
	explicit FileSystemException(const char* message)
		:
		Exception{"XFITSSFIX_SYS_FS", message}
	{}
};

} // namespace

Path get_working_directory()
{
#ifdef XFITSSFIX_WIN32
	// With trailing "nul".
	const auto max_utf16_size = GetCurrentDirectoryW(0, nullptr);

	if (max_utf16_size == 0)
	{
		throw FileSystemException{ErrorMessages::get_working_dir};
	}

	auto utf16_buffer = std::make_unique<WCHAR[]>(max_utf16_size);

	// Without trailing "nul".
	const auto win32_utf16_size = GetCurrentDirectoryW(max_utf16_size, utf16_buffer.get());

	if (win32_utf16_size != (max_utf16_size - 1))
	{
		throw FileSystemException{ErrorMessages::get_working_dir};
	}

	const auto u8_string = encoding::to_utf8(reinterpret_cast<const char16_t*>(utf16_buffer.get()));

	return Path{u8_string.c_str(), static_cast<Int>(u8_string.size())};
#endif // XFITSSFIX_WIN32
}

// --------------------------------------------------------------------------

void create_directory(const char* path)
{
#ifdef XFITSSFIX_WIN32
	const auto u16_path = encoding::to_utf16(path);

	const auto win32_result = CreateDirectoryW(reinterpret_cast<LPCWSTR>(u16_path.c_str()), nullptr);

	if (win32_result == 0)
	{
		const auto last_error = GetLastError();

		if (last_error != ERROR_ALREADY_EXISTS)
		{
			throw FileSystemException{ErrorMessages::create_dir};
		}
	}
#endif // XFITSSFIX_WIN32
}

} // namespace xfitssfix::fs
