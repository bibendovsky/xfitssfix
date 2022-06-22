// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_PROCESS_INCLUDED
#define XFITSSFIX_PROCESS_INCLUDED

#include <xfitssfix/core_types.h>

namespace xfitssfix::process {

bool is_shared_library() noexcept;

Int get_current_id() noexcept;

} // namespace xfitssfix::process

#endif // !XFITSSFIX_PROCESS_INCLUDED
