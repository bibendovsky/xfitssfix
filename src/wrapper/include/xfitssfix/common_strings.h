// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_COMMON_STRINGS_INCLUDED
#define XFITSSFIX_COMMON_STRINGS_INCLUDED

#include <xfitssfix/logger.h>

namespace xfitssfix::common {

struct Strings
{
	static constexpr auto empty = "";
	static constexpr auto less_than_signs_8 = "<<<<<<<<";
	static constexpr auto greater_than_signs_8 = ">>>>>>>>";
};

} // namespace xfitssfix::common

#endif // !XFITSSFIX_COMMON_STRINGS_INCLUDED
