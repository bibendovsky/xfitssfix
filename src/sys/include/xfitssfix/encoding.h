// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_ENCODING_INCLUDED
#define XFITSSFIX_ENCODING_INCLUDED

#include <xfitssfix/string.h>

namespace xfitssfix::encoding {

U16String to_utf16(const char* utf8_string);
U16String to_utf16(const String& utf8_string);

// ==========================================================================

String to_utf8(const char16_t* utf16_string);

} // namespace xfitssfix::encoding

#endif // !XFITSSFIX_ENCODING_INCLUDED
