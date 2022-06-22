// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_AL_API_UTILS_INCLUDED
#define XFITSSFIX_AL_API_UTILS_INCLUDED

#include <xfitssfix/alc.h>
#include <xfitssfix/logger.h>
#include <xfitssfix/string.h>

namespace xfitssfix::al_api {

struct Strings
{
	static constexpr auto indent = "    ";
	static constexpr auto indented_none = "    none";
	static constexpr auto space_equals_space = " = ";
	static constexpr auto three_question_marks = "???";
	static constexpr auto equals_line_16 = "================";
	static constexpr auto null = "<null>";

	static constexpr auto major_version = "major version";
	static constexpr auto minor_version = "minor version";
	static constexpr auto mixer_frequency = "mixer frequency";
	static constexpr auto refresh_interval = "refresh interval";
	static constexpr auto is_synchronous = "is synchronous";
	static constexpr auto mono_sources = "mono sources";
	static constexpr auto stereo_sources = "stereo sources";
	static constexpr auto max_auxiliary_sends = "max auxiliary sends";
};

// ==========================================================================

void log_string(Logger& logger, const char* title, const char* string);

} // namespace xfitssfix::al_api

#endif // !XFITSSFIX_AL_API_UTILS_INCLUDED

