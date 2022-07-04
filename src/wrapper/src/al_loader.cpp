// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <cassert>
#include <xfitssfix/al_loader.h>
#include <xfitssfix/al_symbols.h>
#include <xfitssfix/exception.h>
#include <xfitssfix/string.h>

namespace xfitssfix {

class AlLoaderImplException : public Exception
{
public:
	explicit AlLoaderImplException(const char* message)
		:
		Exception{"AL_LOADER", message}
	{}
};

// ==========================================================================

class AlLoaderImpl : public AlLoader
{
public:
	AlLoaderImpl(SharedLibrary* shared_library);

	void resolve_al_symbols(AlSymbols& al_symbols) override;
	void resolve_efx_symbols(EfxSymbols& efx_symbols) override;
	void resolve_xram_symbols(XRamSymbols& xram_symbols) override;
	void resolve_eax_symbols(EaxSymbols& eax_symbols) override;

private:
	SharedLibrary* shared_library_{};
	LPALGETPROCADDRESS alGetProcAddress_{};

	[[noreturn]] static void fail_resolve_symbol(const char* symbol_name);

	void initialize_al_get_proc_address();

	template<typename T>
	void try_resolve_symbol(const char* symbol_name, T& symbol) noexcept
	{
		assert(shared_library_ != nullptr);
		symbol = reinterpret_cast<T>(shared_library_->resolve(symbol_name));
	}

	template<typename T>
	void resolve_symbol(const char* symbol_name, T& symbol)
	{
		try_resolve_symbol(symbol_name, symbol);

		if (symbol == nullptr)
		{
			fail_resolve_symbol(symbol_name);
		}
	}

	template<typename T>
	void try_resolve_al_symbol(const char* symbol_name, T& symbol)
	{
		assert(alGetProcAddress_ != nullptr);
		symbol = reinterpret_cast<T>(alGetProcAddress_(symbol_name));
	}

	template<typename T>
	void resolve_al_symbol(const char* symbol_name, T& symbol)
	{
		try_resolve_al_symbol(symbol_name, symbol);

		if (symbol == nullptr)
		{
			fail_resolve_symbol(symbol_name);
		}
	}
};

// ==========================================================================

AlLoaderImpl::AlLoaderImpl(xfitssfix::SharedLibrary* shared_library)
{
	if (shared_library == nullptr)
	{
		throw AlLoaderImplException{"Null shared library."};
	}

	shared_library_ = shared_library;
	initialize_al_get_proc_address();
}

[[noreturn]] void AlLoaderImpl::fail_resolve_symbol(const char* symbol_name)
{
	assert(symbol_name != nullptr);
	auto error_message = String{};
	error_message.clear();
	error_message += "Symbol \"";
	error_message += symbol_name;
	error_message += "\" not found.";
	throw AlLoaderImplException{error_message.c_str()};
}

void AlLoaderImpl::initialize_al_get_proc_address()
{
	assert(shared_library_ != nullptr);

	alGetProcAddress_ = reinterpret_cast<decltype(alGetProcAddress_)>(
		shared_library_->resolve(AlSymbolsNames::alGetProcAddress));

	if (alGetProcAddress_ == nullptr)
	{
		fail_resolve_symbol(AlSymbolsNames::alGetProcAddress);
	}
}

void AlLoaderImpl::resolve_al_symbols(AlSymbols& al_symbols)
{
	resolve_symbol(AlSymbolsNames::alcCreateContext, al_symbols.alcCreateContext);
	resolve_symbol(AlSymbolsNames::alcMakeContextCurrent, al_symbols.alcMakeContextCurrent);
	resolve_symbol(AlSymbolsNames::alcProcessContext, al_symbols.alcProcessContext);
	resolve_symbol(AlSymbolsNames::alcSuspendContext, al_symbols.alcSuspendContext);
	resolve_symbol(AlSymbolsNames::alcDestroyContext, al_symbols.alcDestroyContext);
	resolve_symbol(AlSymbolsNames::alcGetCurrentContext, al_symbols.alcGetCurrentContext);
	resolve_symbol(AlSymbolsNames::alcGetContextsDevice, al_symbols.alcGetContextsDevice);
	resolve_symbol(AlSymbolsNames::alcOpenDevice, al_symbols.alcOpenDevice);
	resolve_symbol(AlSymbolsNames::alcCloseDevice, al_symbols.alcCloseDevice);
	resolve_symbol(AlSymbolsNames::alcGetError, al_symbols.alcGetError);
	resolve_symbol(AlSymbolsNames::alcIsExtensionPresent, al_symbols.alcIsExtensionPresent);
	resolve_symbol(AlSymbolsNames::alcGetProcAddress, al_symbols.alcGetProcAddress);
	resolve_symbol(AlSymbolsNames::alcGetEnumValue, al_symbols.alcGetEnumValue);
	resolve_symbol(AlSymbolsNames::alcGetString, al_symbols.alcGetString);
	resolve_symbol(AlSymbolsNames::alcGetIntegerv, al_symbols.alcGetIntegerv);
	resolve_symbol(AlSymbolsNames::alcCaptureOpenDevice, al_symbols.alcCaptureOpenDevice);
	resolve_symbol(AlSymbolsNames::alcCaptureCloseDevice, al_symbols.alcCaptureCloseDevice);
	resolve_symbol(AlSymbolsNames::alcCaptureStart, al_symbols.alcCaptureStart);
	resolve_symbol(AlSymbolsNames::alcCaptureStop, al_symbols.alcCaptureStop);
	resolve_symbol(AlSymbolsNames::alcCaptureSamples, al_symbols.alcCaptureSamples);

	resolve_symbol(AlSymbolsNames::alDopplerFactor, al_symbols.alDopplerFactor);
	resolve_symbol(AlSymbolsNames::alDopplerVelocity, al_symbols.alDopplerVelocity);
	resolve_symbol(AlSymbolsNames::alSpeedOfSound, al_symbols.alSpeedOfSound);
	resolve_symbol(AlSymbolsNames::alDistanceModel, al_symbols.alDistanceModel);
	resolve_symbol(AlSymbolsNames::alEnable, al_symbols.alEnable);
	resolve_symbol(AlSymbolsNames::alDisable, al_symbols.alDisable);
	resolve_symbol(AlSymbolsNames::alIsEnabled, al_symbols.alIsEnabled);
	resolve_symbol(AlSymbolsNames::alGetString, al_symbols.alGetString);
	resolve_symbol(AlSymbolsNames::alGetBooleanv, al_symbols.alGetBooleanv);
	resolve_symbol(AlSymbolsNames::alGetIntegerv, al_symbols.alGetIntegerv);
	resolve_symbol(AlSymbolsNames::alGetFloatv, al_symbols.alGetFloatv);
	resolve_symbol(AlSymbolsNames::alGetDoublev, al_symbols.alGetDoublev);
	resolve_symbol(AlSymbolsNames::alGetBoolean, al_symbols.alGetBoolean);
	resolve_symbol(AlSymbolsNames::alGetInteger, al_symbols.alGetInteger);
	resolve_symbol(AlSymbolsNames::alGetFloat, al_symbols.alGetFloat);
	resolve_symbol(AlSymbolsNames::alGetDouble, al_symbols.alGetDouble);
	resolve_symbol(AlSymbolsNames::alGetError, al_symbols.alGetError);
	resolve_symbol(AlSymbolsNames::alIsExtensionPresent, al_symbols.alIsExtensionPresent);
	resolve_symbol(AlSymbolsNames::alGetProcAddress, al_symbols.alGetProcAddress);
	resolve_symbol(AlSymbolsNames::alGetEnumValue, al_symbols.alGetEnumValue);
	resolve_symbol(AlSymbolsNames::alListenerf, al_symbols.alListenerf);
	resolve_symbol(AlSymbolsNames::alListener3f, al_symbols.alListener3f);
	resolve_symbol(AlSymbolsNames::alListenerfv, al_symbols.alListenerfv);
	resolve_symbol(AlSymbolsNames::alListeneri, al_symbols.alListeneri);
	resolve_symbol(AlSymbolsNames::alListener3i, al_symbols.alListener3i);
	resolve_symbol(AlSymbolsNames::alListeneriv, al_symbols.alListeneriv);
	resolve_symbol(AlSymbolsNames::alGetListenerf, al_symbols.alGetListenerf);
	resolve_symbol(AlSymbolsNames::alGetListener3f, al_symbols.alGetListener3f);
	resolve_symbol(AlSymbolsNames::alGetListenerfv, al_symbols.alGetListenerfv);
	resolve_symbol(AlSymbolsNames::alGetListeneri, al_symbols.alGetListeneri);
	resolve_symbol(AlSymbolsNames::alGetListener3i, al_symbols.alGetListener3i);
	resolve_symbol(AlSymbolsNames::alGetListeneriv, al_symbols.alGetListeneriv);
	resolve_symbol(AlSymbolsNames::alGenSources, al_symbols.alGenSources);
	resolve_symbol(AlSymbolsNames::alDeleteSources, al_symbols.alDeleteSources);
	resolve_symbol(AlSymbolsNames::alIsSource, al_symbols.alIsSource);
	resolve_symbol(AlSymbolsNames::alSourcef, al_symbols.alSourcef);
	resolve_symbol(AlSymbolsNames::alSource3f, al_symbols.alSource3f);
	resolve_symbol(AlSymbolsNames::alSourcefv, al_symbols.alSourcefv);
	resolve_symbol(AlSymbolsNames::alSourcei, al_symbols.alSourcei);
	resolve_symbol(AlSymbolsNames::alSource3i, al_symbols.alSource3i);
	resolve_symbol(AlSymbolsNames::alSourceiv, al_symbols.alSourceiv);
	resolve_symbol(AlSymbolsNames::alGetSourcef, al_symbols.alGetSourcef);
	resolve_symbol(AlSymbolsNames::alGetSource3f, al_symbols.alGetSource3f);
	resolve_symbol(AlSymbolsNames::alGetSourcefv, al_symbols.alGetSourcefv);
	resolve_symbol(AlSymbolsNames::alGetSourcei, al_symbols.alGetSourcei);
	resolve_symbol(AlSymbolsNames::alGetSource3i, al_symbols.alGetSource3i);
	resolve_symbol(AlSymbolsNames::alGetSourceiv, al_symbols.alGetSourceiv);
	resolve_symbol(AlSymbolsNames::alSourcePlayv, al_symbols.alSourcePlayv);
	resolve_symbol(AlSymbolsNames::alSourceStopv, al_symbols.alSourceStopv);
	resolve_symbol(AlSymbolsNames::alSourceRewindv, al_symbols.alSourceRewindv);
	resolve_symbol(AlSymbolsNames::alSourcePausev, al_symbols.alSourcePausev);
	resolve_symbol(AlSymbolsNames::alSourcePlay, al_symbols.alSourcePlay);
	resolve_symbol(AlSymbolsNames::alSourceStop, al_symbols.alSourceStop);
	resolve_symbol(AlSymbolsNames::alSourceRewind, al_symbols.alSourceRewind);
	resolve_symbol(AlSymbolsNames::alSourcePause, al_symbols.alSourcePause);
	resolve_symbol(AlSymbolsNames::alSourceQueueBuffers, al_symbols.alSourceQueueBuffers);
	resolve_symbol(AlSymbolsNames::alSourceUnqueueBuffers, al_symbols.alSourceUnqueueBuffers);
	resolve_symbol(AlSymbolsNames::alGenBuffers, al_symbols.alGenBuffers);
	resolve_symbol(AlSymbolsNames::alDeleteBuffers, al_symbols.alDeleteBuffers);
	resolve_symbol(AlSymbolsNames::alIsBuffer, al_symbols.alIsBuffer);
	resolve_symbol(AlSymbolsNames::alBufferData, al_symbols.alBufferData);
	try_resolve_symbol(AlSymbolsNames::alBufferf, al_symbols.alBufferf);
	try_resolve_symbol(AlSymbolsNames::alBuffer3f, al_symbols.alBuffer3f);
	try_resolve_symbol(AlSymbolsNames::alBufferfv, al_symbols.alBufferfv);
	resolve_symbol(AlSymbolsNames::alBufferi, al_symbols.alBufferi);
	try_resolve_symbol(AlSymbolsNames::alBuffer3i, al_symbols.alBuffer3i);
	try_resolve_symbol(AlSymbolsNames::alBufferiv, al_symbols.alBufferiv);
	resolve_symbol(AlSymbolsNames::alGetBufferf, al_symbols.alGetBufferf);
	try_resolve_symbol(AlSymbolsNames::alGetBuffer3f, al_symbols.alGetBuffer3f);
	try_resolve_symbol(AlSymbolsNames::alGetBufferfv, al_symbols.alGetBufferfv);
	resolve_symbol(AlSymbolsNames::alGetBufferi, al_symbols.alGetBufferi);
	try_resolve_symbol(AlSymbolsNames::alGetBuffer3i, al_symbols.alGetBuffer3i);
	try_resolve_symbol(AlSymbolsNames::alGetBufferiv, al_symbols.alGetBufferiv);
}

void AlLoaderImpl::resolve_efx_symbols(EfxSymbols& efx_symbols)
{
	try_resolve_al_symbol(AlSymbolsNames::alGenEffects, efx_symbols.alGenEffects);
	try_resolve_al_symbol(AlSymbolsNames::alDeleteEffects, efx_symbols.alDeleteEffects);
	try_resolve_al_symbol(AlSymbolsNames::alIsEffect, efx_symbols.alIsEffect);
	try_resolve_al_symbol(AlSymbolsNames::alEffecti, efx_symbols.alEffecti);
	try_resolve_al_symbol(AlSymbolsNames::alEffectiv, efx_symbols.alEffectiv);
	try_resolve_al_symbol(AlSymbolsNames::alEffectf, efx_symbols.alEffectf);
	try_resolve_al_symbol(AlSymbolsNames::alEffectfv, efx_symbols.alEffectfv);
	try_resolve_al_symbol(AlSymbolsNames::alGetEffecti, efx_symbols.alGetEffecti);
	try_resolve_al_symbol(AlSymbolsNames::alGetEffectiv, efx_symbols.alGetEffectiv);
	try_resolve_al_symbol(AlSymbolsNames::alGetEffectf, efx_symbols.alGetEffectf);
	try_resolve_al_symbol(AlSymbolsNames::alGetEffectfv, efx_symbols.alGetEffectfv);
	try_resolve_al_symbol(AlSymbolsNames::alGenFilters, efx_symbols.alGenFilters);
	try_resolve_al_symbol(AlSymbolsNames::alDeleteFilters, efx_symbols.alDeleteFilters);
	try_resolve_al_symbol(AlSymbolsNames::alIsFilter, efx_symbols.alIsFilter);
	try_resolve_al_symbol(AlSymbolsNames::alFilteri, efx_symbols.alFilteri);
	try_resolve_al_symbol(AlSymbolsNames::alFilteriv, efx_symbols.alFilteriv);
	try_resolve_al_symbol(AlSymbolsNames::alFilterf, efx_symbols.alFilterf);
	try_resolve_al_symbol(AlSymbolsNames::alFilterfv, efx_symbols.alFilterfv);
	try_resolve_al_symbol(AlSymbolsNames::alGetFilteri, efx_symbols.alGetFilteri);
	try_resolve_al_symbol(AlSymbolsNames::alGetFilteriv, efx_symbols.alGetFilteriv);
	try_resolve_al_symbol(AlSymbolsNames::alGetFilterf, efx_symbols.alGetFilterf);
	try_resolve_al_symbol(AlSymbolsNames::alGetFilterfv, efx_symbols.alGetFilterfv);
	try_resolve_al_symbol(AlSymbolsNames::alGenAuxiliaryEffectSlots, efx_symbols.alGenAuxiliaryEffectSlots);
	try_resolve_al_symbol(AlSymbolsNames::alDeleteAuxiliaryEffectSlots, efx_symbols.alDeleteAuxiliaryEffectSlots);
	try_resolve_al_symbol(AlSymbolsNames::alIsAuxiliaryEffectSlot, efx_symbols.alIsAuxiliaryEffectSlot);
	try_resolve_al_symbol(AlSymbolsNames::alAuxiliaryEffectSloti, efx_symbols.alAuxiliaryEffectSloti);
	try_resolve_al_symbol(AlSymbolsNames::alAuxiliaryEffectSlotiv, efx_symbols.alAuxiliaryEffectSlotiv);
	try_resolve_al_symbol(AlSymbolsNames::alAuxiliaryEffectSlotf, efx_symbols.alAuxiliaryEffectSlotf);
	try_resolve_al_symbol(AlSymbolsNames::alAuxiliaryEffectSlotfv, efx_symbols.alAuxiliaryEffectSlotfv);
	try_resolve_al_symbol(AlSymbolsNames::alGetAuxiliaryEffectSloti, efx_symbols.alGetAuxiliaryEffectSloti);
	try_resolve_al_symbol(AlSymbolsNames::alGetAuxiliaryEffectSlotiv, efx_symbols.alGetAuxiliaryEffectSlotiv);
	try_resolve_al_symbol(AlSymbolsNames::alGetAuxiliaryEffectSlotf, efx_symbols.alGetAuxiliaryEffectSlotf);
	try_resolve_al_symbol(AlSymbolsNames::alGetAuxiliaryEffectSlotfv, efx_symbols.alGetAuxiliaryEffectSlotfv);
}

void AlLoaderImpl::resolve_xram_symbols(XRamSymbols& xram_symbols)
{
	try_resolve_al_symbol(XRamSymbolsNames::EAXSetBufferMode, xram_symbols.EAXSetBufferMode);
	try_resolve_al_symbol(XRamSymbolsNames::EAXGetBufferMode, xram_symbols.EAXGetBufferMode);
}

void AlLoaderImpl::resolve_eax_symbols(EaxSymbols& eax_symbols)
{
	try_resolve_al_symbol(EaxSymbolsNames::EAXSet, eax_symbols.EAXSet);
	try_resolve_al_symbol(EaxSymbolsNames::EAXGet, eax_symbols.EAXGet);
}

// ==========================================================================

AlLoaderUPtr make_al_loader(SharedLibrary* shared_library)
{
	return std::make_unique<AlLoaderImpl>(shared_library);
}

} // namespace xfitssfix
