// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <cassert>
#include <algorithm>
#include <exception>
#include <xfitssfix/al.h>
#include <xfitssfix/al_api_utils.h>

namespace xfitssfix::al_api
{

void log_string(Logger& logger, const char* title, const char* string)
{
	assert(title != nullptr);
	auto message = String{};
	message.reserve(128);
	message += title;
	message += ": ";
	message += (string ? string : Strings::null);
	logger.info(message.c_str());
}

} // namespace xfitssfix::al_api
