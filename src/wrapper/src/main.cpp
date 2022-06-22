// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT


#include <xfitssfix/main.h>

#include <windows.h>

#include <xfitssfix/al_api.h>


BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,
	DWORD fdwReason,
	LPVOID lpvReserved)
{
	static_cast<void>(hinstDLL);
	static_cast<void>(lpvReserved);

	switch (fdwReason)
	{
		case DLL_PROCESS_DETACH:
			xfitssfix::g_al_api.on_process_detach();
			break;

		case DLL_THREAD_DETACH:
			xfitssfix::g_al_api.on_thread_detach();
			break;

		default:
			break;
	}

	return TRUE;
}
