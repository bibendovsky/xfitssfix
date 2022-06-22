// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_PLATFORM_INCLUDED
#define XFITSSFIX_PLATFORM_INCLUDED

#ifdef _WIN32

#ifndef XFITSSFIX_WIN32
#define XFITSSFIX_WIN32
#endif // !XFITSSFIX_WIN32

#else
#error Unsupported platform
#endif // _WIN32

namespace xfitssfix::platform {

bool is_vista_or_higher() noexcept;

} // namespace xfitssfix::platform

#endif // !XFITSSFIX_PLATFORM_INCLUDED
