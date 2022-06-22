// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_AL_LOADER_INCLUDED
#define XFITSSFIX_AL_LOADER_INCLUDED

#include <memory>
#include <xfitssfix/al_symbols.h>
#include <xfitssfix/shared_library.h>

namespace xfitssfix {

class AlLoader
{
public:
	AlLoader() noexcept = default;
	virtual ~AlLoader() = default;

	virtual void resolve_al_symbols(AlSymbols& al_symbols) = 0;
	virtual void resolve_efx_symbols(EfxSymbols& efx_symbols) = 0;
	virtual void resolve_xram_symbols(XRamSymbols& xram_symbols) = 0;
	virtual void resolve_eax_symbols(EaxSymbols& eax_symbols) = 0;
};

// --------------------------------------------------------------------------

using AlLoaderUPtr = std::unique_ptr<AlLoader>;

AlLoaderUPtr make_al_loader(SharedLibrary* shared_library);

} // namespace xfitssfix

#endif // !XFITSSFIX_AL_LOADER_INCLUDED
