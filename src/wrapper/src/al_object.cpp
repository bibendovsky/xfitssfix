// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <cassert>
#include <xfitssfix/al_object.h>
#include <xfitssfix/al_symbols.h>
#include <xfitssfix/exception.h>

namespace xfitssfix {

class AlObjectException : public Exception
{
public:
	explicit AlObjectException(const char* message)
		:
		Exception{"AL_OBJECT", message}
	{}
};

// ==========================================================================

namespace {

[[noreturn]] void al_object_fail(const char* message)
{
	throw AlObjectException{message};
}

} // namespace

} // namespace xfitssfix
