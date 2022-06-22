// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <windows.h>
#include <type_traits>
#include <xfitssfix/process.h>

namespace xfitssfix::process {

bool is_shared_library() noexcept
{
	const auto root_module = GetModuleHandleW(nullptr);
	auto current_module = HMODULE{};

	const auto winapi_result = GetModuleHandleExW(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		reinterpret_cast<LPCWSTR>(is_shared_library),
		&current_module);

	if (winapi_result == FALSE)
	{
		return false;
	}

	return root_module != current_module;
}

// ==========================================================================

Int get_current_id() noexcept
{
	return static_cast<std::make_signed_t<DWORD>>(GetCurrentProcessId());
}

} // namespace xfitssfix::process
