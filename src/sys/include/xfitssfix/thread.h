// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_THREAD_INCLUDED
#define XFITSSFIX_THREAD_INCLUDED

#include <memory>

namespace xfitssfix::thread {

void set_name(const char* utf8_name);

} // namespace xfitssfix::thread

namespace xfitssfix::this_thread {

void sleep_for_ms(int milliseconds) noexcept;

} // namespace xfitssfix::this_thread

namespace xfitssfix {

class Thread
{
public:
	Thread() = default;
	virtual ~Thread() = default;

	virtual bool is_joinable() const noexcept = 0;
	virtual void join() = 0;
};

// --------------------------------------------------------------------------

using ThreadUPtr = std::unique_ptr<Thread>;

using ThreadFunction = void (*)(void* user_data);

ThreadUPtr make_thread(ThreadFunction thread_function, void* user_data);

} // namespace xfitssfix

#endif // !XFITSSFIX_THREAD_INCLUDED
