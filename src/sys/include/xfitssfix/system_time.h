// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_SYSTEM_TIME_INCLUDED
#define XFITSSFIX_SYSTEM_TIME_INCLUDED

#include "string.h"

namespace xfitssfix {

struct SystemTime
{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int millisecond;
};

SystemTime make_system_time() noexcept;

void make_system_time_string(const SystemTime& system_time, String& system_time_string);

} // namespace xfitssfix

#endif // !XFITSSFIX_SYSTEM_TIME_INCLUDED
