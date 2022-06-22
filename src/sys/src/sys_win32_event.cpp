// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <xfitssfix/exception.h>
#include <xfitssfix/sys_win32_event.h>

namespace xfitssfix {

class Win32EventException : public Exception
{
public:
	explicit Win32EventException(const char* message)
		:
		Exception{"SYS_WIN32_EVENT", message}
	{}
};

// --------------------------------------------------------------------------

SysWin32Event::SysWin32Event()
	:
	handle_{make_win32_event()}
{}

SysWin32Event::~SysWin32Event()
{
	CloseHandle(handle_);
}

void SysWin32Event::set(bool value)
{
	BOOL win32_result;

	if (value)
	{
		win32_result = SetEvent(handle_);
	}
	else
	{
		win32_result = ResetEvent(handle_);
	}

	if (win32_result == FALSE)
	{
		throw Win32EventException("Failed to set a value.");
	}
}

void SysWin32Event::wait()
{
	wait_for_ms_internal(INFINITE);
}

bool SysWin32Event::wait_for(std::chrono::milliseconds timeout_ms)
{
	DWORD win32_timeout_ms;

	if (timeout_ms.count() >= INFINITE)
	{
		win32_timeout_ms = INFINITE;
	}
	else
	{
		win32_timeout_ms = static_cast<DWORD>(timeout_ms.count());
	}

	return wait_for_ms_internal(win32_timeout_ms);
}

HANDLE SysWin32Event::make_win32_event()
{
	const auto handle = CreateEventW(nullptr, TRUE, FALSE, nullptr);

	if (handle == nullptr)
	{
		throw Win32EventException("Failed to create.");
	}

	return handle;
}

bool SysWin32Event::wait_for_ms_internal(DWORD timeout_ms)
{
	const auto win32_result = WaitForSingleObject(handle_, static_cast<DWORD>(timeout_ms));

	switch (win32_result)
	{
		case WAIT_OBJECT_0: return true;
		case WAIT_TIMEOUT: return false;
		default: throw Win32EventException("Wait failed.");
	}
}

} // namespace xfitssfix
