// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_UTILS_INCLUDED
#define XFITSSFIX_UTILS_INCLUDED

#include <xfitssfix/logger.h>

namespace xfitssfix::utils {

void log_exception(Logger& logger, const char* message = nullptr) noexcept;

} // namespace xfitssfix::utils

#endif // !XFITSSFIX_UTILS_INCLUDED
