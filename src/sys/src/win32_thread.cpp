// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <cassert>
#include <windows.h>
#include <process.h>
#include <xfitssfix/encoding.h>
#include <xfitssfix/exception.h>
#include <xfitssfix/shared_library.h>
#include <xfitssfix/string.h>
#include <xfitssfix/thread.h>

namespace xfitssfix::thread {

void set_name(const char* utf8_name)
{
	assert(utf8_name != nullptr);
	auto shared_library = make_shared_library("kernel32.dll");

	using SetThreadDescriptionFunc = HRESULT(WINAPI*)(
		HANDLE hThread,
		PCWSTR lpThreadDescription);

	const auto set_thread_description_func =
		reinterpret_cast<SetThreadDescriptionFunc>(
			shared_library->resolve("SetThreadDescription"));

	if (set_thread_description_func == nullptr)
	{
		return;
	}

	const auto thread_handle = GetCurrentThread();
	const auto& utf16_name = encoding::to_utf16(utf8_name);
	set_thread_description_func(thread_handle, reinterpret_cast<LPCWSTR>(utf16_name.c_str()));
}

} // namespace xfitssfix::thread

// --------------------------------------------------------------------------

namespace xfitssfix::this_thread {

void sleep_for_ms(int milliseconds) noexcept
{
	assert(milliseconds >= 0);
	Sleep(static_cast<DWORD>(milliseconds));
}

} // namespace xfitssfix::this_thread

// --------------------------------------------------------------------------

namespace xfitssfix {

class ThreadException : public Exception
{
public:
	explicit ThreadException(const char* message)
		:
		Exception{"WIN32_THREAD", message}
	{}
};

// --------------------------------------------------------------------------

class ThreadImpl final : public Thread
{
public:
	ThreadImpl(ThreadFunction thread_function, void* user_data);
	~ThreadImpl() override;

	bool is_joinable() const noexcept override;
	void join() override;

private:
	HANDLE handle_;
	ThreadFunction thread_function_{};
	void* user_data_{};

	[[noreturn]] void fail(const char* message);

	static unsigned int __stdcall proxy_thread_function(void* user_data);
	void thread_function();
}; // ThreadImpl

// --------------------------------------------------------------------------

ThreadImpl::ThreadImpl(ThreadFunction thread_function, void* user_data)
	:
	thread_function_{thread_function},
	user_data_{user_data}
{
	if (thread_function == nullptr)
	{
		fail("Null function.");
	}

	constexpr auto stack_size = 1U << 20;

	handle_ = reinterpret_cast<HANDLE>(_beginthreadex(
		nullptr,
		stack_size,
		proxy_thread_function,
		this,
		CREATE_SUSPENDED,
		nullptr));

	if (handle_ == nullptr)
	{
		fail("Failed to create.");
	}

	const auto resume_result = ResumeThread(handle_);
	constexpr auto invalid_resume_result = ~DWORD{};

	if (resume_result == invalid_resume_result)
	{
		fail("Failed to resume.");
	}
}

bool ThreadImpl::is_joinable() const noexcept
{
	return handle_ != nullptr;
}

void ThreadImpl::join()
{
	if (!is_joinable())
	{
		fail("Non-joinable.");
	}

	WaitForSingleObject(handle_, INFINITE);
	CloseHandle(handle_);
	handle_ = nullptr;
}

ThreadImpl::~ThreadImpl()
{
	if (!is_joinable())
	{
		return;
	}

	join();
}

[[noreturn]] void ThreadImpl::fail(const char* message)
{
	throw ThreadException{message};
}

unsigned int __stdcall ThreadImpl::proxy_thread_function(void* user_data)
{
	auto thread = static_cast<ThreadImpl*>(user_data);

	try
	{
		thread->thread_function();
	}
	catch (...)
	{
	}

	_endthreadex(0);
	return 0;
}

void ThreadImpl::thread_function()
{
	thread_function_(user_data_);
}

// --------------------------------------------------------------------------

ThreadUPtr make_thread(ThreadFunction thread_function, void* user_data)
{
	return std::make_unique<ThreadImpl>(thread_function, user_data);
}

} // namespace xfitssfix
