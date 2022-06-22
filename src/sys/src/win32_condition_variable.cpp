// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <mutex>
#include <windows.h>
#include <xfitssfix/condition_variable.h>
#include <xfitssfix/exception.h>
#include <xfitssfix/platform.h>
#include <xfitssfix/scope_exit.h>
#include <xfitssfix/shared_library.h>
#include <xfitssfix/sys_win32_critical_section.h>
#include <xfitssfix/sys_win32_condition_variable.h>
#include <xfitssfix/sys_win32_event.h>

namespace xfitssfix {

class Win32ConditionVariableImplException : public Exception
{
public:
	explicit Win32ConditionVariableImplException(const char* message)
		:
		Exception{"WIN32_CONDITION_VARIABLE", message}
	{}
};

// --------------------------------------------------------------------------

class ConditionVariable::Impl
{
public:
	Impl() = default;
	virtual ~Impl() = default;

	virtual void wait(UniqueLock& unique_lock) = 0;
	virtual bool wait_for(UniqueLock& unique_lock, std::chrono::milliseconds timeout_ms) = 0;
	virtual void notify_one() = 0;
};

// --------------------------------------------------------------------------

class ConditionVariable::ImplCs final : public Impl
{
public:
	void wait(UniqueLock& unique_lock) override;
	bool wait_for(UniqueLock& unique_lock, std::chrono::milliseconds timeout_ms) override;
	void notify_one() override;

private:
	bool is_waiting_{};
	SysWin32CriticalSection win32_critical_section_{};
	SysWin32Event win32_event_{};
};

// --------------------------------------------------------------------------

void ConditionVariable::ImplCs::wait(UniqueLock& unique_lock)
{
	wait_for(unique_lock, std::chrono::milliseconds{INFINITE});
}

bool ConditionVariable::ImplCs::wait_for(UniqueLock& unique_lock, std::chrono::milliseconds timeout_ms)
{
	if (!unique_lock.owns_lock())
	{
		throw Win32ConditionVariableImplException{"Expected locked mutex."};
	}

	const auto lock = std::scoped_lock{win32_critical_section_};

	if (is_waiting_)
	{
		throw Win32ConditionVariableImplException{"Already waiting."};
	}

	is_waiting_ = true;

	const auto is_waiting_scope_exit = ScopeExit{
		[this]()
		{
			is_waiting_ = false;
		}};

	bool wait_result;

	{
		unique_lock.unlock();

		const auto unique_lock_scope_exit = ScopeExit{
			[&unique_lock]()
			{
				unique_lock.lock();
			}};

		wait_result = win32_event_.wait_for(timeout_ms);
	}

	win32_event_.set(false);

	return wait_result;
}

void ConditionVariable::ImplCs::notify_one()
{
	win32_event_.set(true);
}

// --------------------------------------------------------------------------

class ConditionVariable::ImplSrw final : public Impl
{
public:
	void wait(UniqueLock& unique_lock) override;
	bool wait_for(UniqueLock& unique_lock, std::chrono::milliseconds timeout_ms) override;
	void notify_one() override;

private:
	SysWin32ConditionVariable win32_condition_variable_;
};

// --------------------------------------------------------------------------

void ConditionVariable::ImplSrw::wait(UniqueLock& unique_lock)
{
	wait_for(unique_lock, std::chrono::milliseconds{INFINITE});
}

bool ConditionVariable::ImplSrw::wait_for(UniqueLock& unique_lock, std::chrono::milliseconds timeout_ms)
{
	auto& win32_critical_section = *static_cast<SysWin32CriticalSection*>(unique_lock.mutex()->native_handle());
	return win32_condition_variable_.sleep(win32_critical_section, timeout_ms);
}

void ConditionVariable::ImplSrw::notify_one()
{
	win32_condition_variable_.wake();
}

// --------------------------------------------------------------------------

ConditionVariable::ConditionVariable()
{
	if (platform::is_vista_or_higher())
	{
		impl_ = std::make_unique<ImplSrw>();
	}
	else
	{
		impl_ = std::make_unique<ImplCs>();
	}
}

ConditionVariable::~ConditionVariable() = default;

void ConditionVariable::wait(UniqueLock& unique_lock)
{
	wait_for(unique_lock, std::chrono::milliseconds{INFINITE});
}

bool ConditionVariable::wait_for(UniqueLock& unique_lock, std::chrono::milliseconds timeout_ms)
{
	return impl_->wait_for(unique_lock, timeout_ms);
}

void ConditionVariable::notify_one()
{
	impl_->notify_one();
}

} // namespace xfitssfix
