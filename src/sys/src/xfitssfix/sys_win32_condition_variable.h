// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_SYS_WIN32_CONDITION_VARIABLE_INCLUDED
#define XFITSSFIX_SYS_WIN32_CONDITION_VARIABLE_INCLUDED

#include <chrono>
#include <windows.h>
#include <xfitssfix/shared_library.h>
#include <xfitssfix/sys_win32_critical_section.h>

namespace xfitssfix {

class SysWin32ConditionVariable
{
public:
	SysWin32ConditionVariable();
	SysWin32ConditionVariable(const SysWin32ConditionVariable& rhs) = delete;
	SysWin32ConditionVariable& operator=(const SysWin32ConditionVariable& rhs) = delete;
	~SysWin32ConditionVariable();

	bool sleep(SysWin32CriticalSection& critical_section, std::chrono::milliseconds timeout);
	void wake();

private:
	using InitializeConditionVariableFunc = void (WINAPI*)(PCONDITION_VARIABLE ConditionVariable);
	using WakeConditionVariableFunc = void (WINAPI*)(PCONDITION_VARIABLE ConditionVariable);

	using SleepConditionVariableCSFunc = BOOL(WINAPI*)(
		PCONDITION_VARIABLE ConditionVariable,
		PCRITICAL_SECTION CriticalSection,
		DWORD dwMilliseconds);

	SharedLibraryUPtr kernel32_shared_library_{};
	InitializeConditionVariableFunc initialize_condition_variable_func_{};
	WakeConditionVariableFunc wake_condition_variable_func_{};
	SleepConditionVariableCSFunc sleep_condition_variable_cs_func_{};

	CONDITION_VARIABLE condition_variable_;
};

} // namespace xfitssfix

#endif // !XFITSSFIX_SYS_WIN32_CONDITION_VARIABLE_INCLUDED
