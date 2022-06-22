// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <xfitssfix/exception.h>
#include <xfitssfix/sys_win32_condition_variable.h>

namespace xfitssfix {

class SysWin32ConditionVariableException : public Exception
{
public:
	explicit SysWin32ConditionVariableException( const char* message)
		:
		Exception{"SYS_WIN32_CONDITION_VARIABLE", message}
	{}
};

// --------------------------------------------------------------------------

SysWin32ConditionVariable::SysWin32ConditionVariable()
	:
	kernel32_shared_library_{make_shared_library("kernel32")},
	initialize_condition_variable_func_{
		reinterpret_cast<InitializeConditionVariableFunc>(
			kernel32_shared_library_->resolve("InitializeConditionVariable"))},
	wake_condition_variable_func_{
		reinterpret_cast<WakeConditionVariableFunc>(
			kernel32_shared_library_->resolve("WakeConditionVariable"))},
	sleep_condition_variable_cs_func_{
		reinterpret_cast<SleepConditionVariableCSFunc>(
			kernel32_shared_library_->resolve("SleepConditionVariableCS"))}
{
	if (initialize_condition_variable_func_ == nullptr ||
		wake_condition_variable_func_ == nullptr ||
		sleep_condition_variable_cs_func_ == nullptr)
	{
		throw SysWin32ConditionVariableException{"Missing symbols."};
	}

	initialize_condition_variable_func_(&condition_variable_);
}

SysWin32ConditionVariable::~SysWin32ConditionVariable() = default;

bool SysWin32ConditionVariable::sleep(SysWin32CriticalSection& critical_section, std::chrono::milliseconds timeout)
{
	DWORD timeout_ms;

	if (timeout.count() >= INFINITE)
	{
		timeout_ms = INFINITE;
	}
	else
	{
		timeout_ms = static_cast<DWORD>(timeout.count());
	}

	const auto win32_result = sleep_condition_variable_cs_func_(
		&condition_variable_,
		&critical_section.get(),
		timeout_ms);

	return win32_result != 0;
}

void SysWin32ConditionVariable::wake()
{
	wake_condition_variable_func_(&condition_variable_);
}

} // namespace xfitssfix
