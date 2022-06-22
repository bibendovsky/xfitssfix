// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_CONDITION_VARIABLE_INCLUDED
#define XFITSSFIX_CONDITION_VARIABLE_INCLUDED

#include <chrono>
#include <memory>
#include <mutex>
#include <xfitssfix/mutex.h>

namespace xfitssfix {

class ConditionVariable
{
public:
	using UniqueLock = std::unique_lock<Mutex>;

	ConditionVariable();
	~ConditionVariable();

	void wait(UniqueLock& unique_lock);
	bool wait_for(UniqueLock& unique_lock, std::chrono::milliseconds timeout_ms);
	void notify_one();

private:
	class Impl;
	class ImplCs;
	class ImplSrw;

	using ImplUPtr = std::unique_ptr<Impl>;
	ImplUPtr impl_;
};

} // namespace xfitssfix

#endif // !XFITSSFIX_CONDITION_VARIABLE_INCLUDED
