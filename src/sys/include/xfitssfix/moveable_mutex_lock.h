// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_MOVEABLE_MUTEX_LOCK_INCLUDED
#define XFITSSFIX_MOVEABLE_MUTEX_LOCK_INCLUDED

namespace xfitssfix {

class Mutex;

// --------------------------------------------------------------------------

class MoveableMutexLock
{
public:
	MoveableMutexLock() noexcept;
	explicit MoveableMutexLock(Mutex& mutex) noexcept;
	MoveableMutexLock(const MoveableMutexLock& rhs) = delete;
	MoveableMutexLock(MoveableMutexLock&& rhs) noexcept;
	~MoveableMutexLock();

private:
	Mutex* mutex_{};
};

} // namespace xfitssfix

#endif // XFITSSFIX_MOVEABLE_MUTEX_LOCK_INCLUDED
