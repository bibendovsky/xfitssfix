// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <memory>
#include <windows.h>
#include <xfitssfix/encoding.h>
#include <xfitssfix/exception.h>
#include <xfitssfix/shared_library.h>

namespace xfitssfix {

class Win32SharedLibraryException : public Exception
{
public:
	explicit Win32SharedLibraryException(const char* message)
		:
		Exception{"WIN32_SHARED_LIBRARY", message}
	{}
};

// --------------------------------------------------------------------------

class Win32SharedLibrary : public SharedLibrary
{
public:
	Win32SharedLibrary(const char* path);
	~Win32SharedLibrary() override;

	void* resolve(const char* symbol_name) noexcept override;

private:
	HMODULE win32_module_;
};

// --------------------------------------------------------------------------

Win32SharedLibrary::Win32SharedLibrary(const char* path)
	:
	win32_module_{}
{
	const auto u16_path = encoding::to_utf16(path);
	win32_module_ = LoadLibraryW(reinterpret_cast<LPCWSTR>(u16_path.c_str()));

	if (win32_module_ == nullptr)
	{
		throw Win32SharedLibraryException{"::LoadLibraryW failed."};
	}
}

Win32SharedLibrary::~Win32SharedLibrary()
{
	if (win32_module_ != nullptr)
	{
		static_cast<void>(FreeLibrary(win32_module_));
	}
}

void* Win32SharedLibrary::resolve(const char* symbol_name) noexcept
{
	return reinterpret_cast<void*>(GetProcAddress(win32_module_, symbol_name));
}

// --------------------------------------------------------------------------

SharedLibraryUPtr make_shared_library(
	const char* path)
{
	return std::make_unique<Win32SharedLibrary>(path);
}

} // namespace xfitssfix
