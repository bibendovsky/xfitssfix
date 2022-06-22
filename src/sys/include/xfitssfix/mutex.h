// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_MUTEX_INCLUDED
#define XFITSSFIX_MUTEX_INCLUDED

#include <memory>

namespace xfitssfix {

class Mutex
{
public:
	Mutex() noexcept;
	virtual ~Mutex();

	virtual void lock() = 0;
	virtual void unlock() = 0;
	virtual void* native_handle() noexcept = 0;
};

// --------------------------------------------------------------------------

using MutexUPtr = std::unique_ptr<Mutex>;

MutexUPtr make_mutex();

} // namespace xfitssfix

#endif // !XFITSSFIX_MUTEX_INCLUDED
