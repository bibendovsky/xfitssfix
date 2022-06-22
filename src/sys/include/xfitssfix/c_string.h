// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_C_STRING_INCLUDED
#define XFITSSFIX_C_STRING_INCLUDED

#include <cassert>
#include <xfitssfix/core_types.h>

namespace xfitssfix::c_string {

template<typename TChar>
inline constexpr Int get_size(const TChar* c_string) noexcept
{
	assert(c_string);
	auto size = Int{};

	while (c_string[size] != TChar{})
	{
		size += 1;
	}

	return size;
}

} // namespace xfitssfix::c_string

#endif // !XFITSSFIX_C_STRING_INCLUDED
