// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_ENV_INCLUDED
#define XFITSSFIX_ENV_INCLUDED

#include <xfitssfix/fs_path.h>

namespace xfitssfix::env {

enum class SpecialFolderType
{
	none = 0,
	app_data,
};

// --------------------------------------------------------------------------

fs::Path get_special_folder(SpecialFolderType special_folder_type);

} // namespace xfitssfix::env

#endif // !XFITSSFIX_ENV_INCLUDED
