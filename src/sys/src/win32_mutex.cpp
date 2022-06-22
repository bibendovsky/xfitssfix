// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <xfitssfix/mutex.h>
#include <xfitssfix/sys_win32_critical_section.h>

namespace xfitssfix {

class Win32Mutex final : public Mutex
{
public:
	// ======================================================================
	// Mutex

	void lock() override;
	void unlock() override;
	void* native_handle() noexcept override;

	// Mutex
	// ======================================================================

private:
	SysWin32CriticalSection win32_critical_section_{};
};

// --------------------------------------------------------------------------

void Win32Mutex::lock()
{
	win32_critical_section_.lock();
}

void Win32Mutex::unlock()
{
	win32_critical_section_.unlock();
}

void* Win32Mutex::native_handle() noexcept
{
	return &win32_critical_section_;
}

// --------------------------------------------------------------------------

Mutex::Mutex() noexcept = default;

Mutex::~Mutex() = default;

// --------------------------------------------------------------------------

MutexUPtr make_mutex()
{
	return std::make_unique<Win32Mutex>();
}

} // namespace xfitssfix
