// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_SYS_WIN32_EVENT_INCLUDED
#define XFITSSFIX_SYS_WIN32_EVENT_INCLUDED

#include <chrono>
#include <windows.h>

namespace xfitssfix {

class SysWin32Event
{
public:
	SysWin32Event();
	SysWin32Event(const SysWin32Event& rhs) = delete;
	SysWin32Event& operator=(const SysWin32Event& rhs) = delete;
	~SysWin32Event();

	void set(bool value);
	void wait();
	bool wait_for(std::chrono::milliseconds timeout_ms);

private:
	HANDLE handle_{};

	static HANDLE make_win32_event();
	bool wait_for_ms_internal(DWORD timeout_ms);
};

} // namespace xfitssfix

#endif // !XFITSSFIX_SYS_WIN32_EVENT_INCLUDED
