// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_SHARED_LIBRARY_INCLUDED
#define XFITSSFIX_SHARED_LIBRARY_INCLUDED

#include <memory>

namespace xfitssfix {

class SharedLibrary
{
public:
	SharedLibrary() = default;
	virtual ~SharedLibrary() = default;

	virtual void* resolve(const char* symbol_name) noexcept = 0;
};

// --------------------------------------------------------------------------

using SharedLibraryUPtr = std::unique_ptr<SharedLibrary>;

SharedLibraryUPtr make_shared_library(const char* path);

} // namespace xfitssfix

#endif // !XFITSSFIX_SHARED_LIBRARY_INCLUDED
