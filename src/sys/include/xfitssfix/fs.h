// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_FS_INCLUDED
#define XFITSSFIX_FS_INCLUDED

#include <xfitssfix/fs_path.h>

namespace xfitssfix::fs {

Path get_working_directory();

void create_directory(const char* path);

} // namespace xfitssfix::fs

#endif // !XFITSSFIX_FS_INCLUDED
