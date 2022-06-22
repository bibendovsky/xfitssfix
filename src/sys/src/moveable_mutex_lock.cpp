// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <utility>
#include <xfitssfix/moveable_mutex_lock.h>
#include <xfitssfix/mutex.h>

namespace xfitssfix {

MoveableMutexLock::MoveableMutexLock() noexcept = default;

MoveableMutexLock::MoveableMutexLock(Mutex& mutex) noexcept
	:
	mutex_{&mutex}
{
	mutex_->lock();
}

MoveableMutexLock::MoveableMutexLock(MoveableMutexLock&& rhs) noexcept
{
	std::swap(mutex_, rhs.mutex_);
}

MoveableMutexLock::~MoveableMutexLock()
{
	if (mutex_)
	{
		mutex_->unlock();
	}
}

} // namespace xfitssfix
