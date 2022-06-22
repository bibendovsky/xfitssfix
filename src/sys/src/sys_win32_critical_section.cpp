// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <xfitssfix/sys_win32_critical_section.h>

namespace xfitssfix {

SysWin32CriticalSection::SysWin32CriticalSection()
{
	InitializeCriticalSection(&critical_section_);
}

SysWin32CriticalSection::~SysWin32CriticalSection()
{
	DeleteCriticalSection(&critical_section_);
}

void SysWin32CriticalSection::lock()
{
	EnterCriticalSection(&critical_section_);
}

void SysWin32CriticalSection::unlock()
{
	LeaveCriticalSection(&critical_section_);
}

CRITICAL_SECTION& SysWin32CriticalSection::get() noexcept
{
	return critical_section_;
}

} // namespace xfitssfix
