// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_AL_API_INCLUDED
#define XFITSSFIX_AL_API_INCLUDED

namespace xfitssfix {

class AlApi
{
public:
	AlApi() noexcept = default;
	virtual ~AlApi() = default;

	virtual void on_thread_detach() noexcept = 0;
	virtual void on_process_detach() noexcept = 0;
};

// ==========================================================================

extern AlApi& g_al_api;

} // namespace xfitssfix

#endif // !XFITSSFIX_AL_API_INCLUDED
