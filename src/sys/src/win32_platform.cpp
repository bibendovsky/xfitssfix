// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <xfitssfix/platform.h>
#include <windows.h>

namespace xfitssfix::platform {

bool win32_is_vista_or_higher_internal() noexcept
{
	constexpr auto win32_type_mask = DWORD{VER_MAJORVERSION | VER_MINORVERSION};

	auto os_version_info_w = OSVERSIONINFOEXW{};
	os_version_info_w.dwOSVersionInfoSize = static_cast<DWORD>(sizeof(::OSVERSIONINFOEXW));
	os_version_info_w.dwMajorVersion = 6;
	os_version_info_w.dwMinorVersion = 0;

	const auto win32_condition_mask = VerSetConditionMask(0, win32_type_mask, VER_GREATER_EQUAL);

	const auto win32_result = VerifyVersionInfoW(
		&os_version_info_w,
		win32_type_mask,
		win32_condition_mask);

	return win32_result != FALSE;
}

namespace {

bool is_vista_or_higher_is_initialized_{};
bool is_vista_or_higher_{};

} // namespace

bool is_vista_or_higher() noexcept
{
	if (!is_vista_or_higher_is_initialized_)
	{
		is_vista_or_higher_is_initialized_ = true;
		is_vista_or_higher_ = win32_is_vista_or_higher_internal();
	}

	return is_vista_or_higher_;
}

} // namespace xfitssfix::platform
