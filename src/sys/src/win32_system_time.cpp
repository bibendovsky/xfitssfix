// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <windows.h>
#include <xfitssfix/system_time.h>

namespace xfitssfix {

SystemTime make_system_time() noexcept
{
	SYSTEMTIME win32_system_time;
	GetLocalTime(&win32_system_time);
	auto system_time = SystemTime{};
	system_time.year = win32_system_time.wYear;
	system_time.month = win32_system_time.wMonth;
	system_time.day = win32_system_time.wDay;
	system_time.hour = win32_system_time.wHour;
	system_time.minute = win32_system_time.wMinute;
	system_time.second = win32_system_time.wSecond;
	system_time.millisecond = win32_system_time.wMilliseconds;
	return system_time;
}

} // namespace xfitssfix
