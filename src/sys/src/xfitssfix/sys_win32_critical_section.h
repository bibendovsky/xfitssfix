// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_SYS_WIN32_CRITICAL_SECTION_INCLUDED
#define XFITSSFIX_SYS_WIN32_CRITICAL_SECTION_INCLUDED

#include <windows.h>

namespace xfitssfix {

class SysWin32CriticalSection
{
public:
	SysWin32CriticalSection();
	SysWin32CriticalSection(const SysWin32CriticalSection& rhs) = delete;
	SysWin32CriticalSection& operator=(const SysWin32CriticalSection& rhs) = delete;
	~SysWin32CriticalSection();

	void lock();
	void unlock();

	CRITICAL_SECTION& get() noexcept;

private:
	CRITICAL_SECTION critical_section_;
};

} // namespace xfitssfix

#endif // !XFITSSFIX_SYS_WIN32_CRITICAL_SECTION_INCLUDED
