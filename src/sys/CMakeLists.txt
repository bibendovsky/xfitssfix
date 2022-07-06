# XFITSSFIX - X-Fi Titanium Streaming Sound Fix
# Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15.7 FATAL_ERROR)
project(xfitssfix_sys VERSION 0.9.2 LANGUAGES CXX)

if (NOT WIN32)
	message(FATAL_ERROR "Unsupported platform.")
endif ()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(make_version_string)

make_version_string(XFITSSFIX_SYS_VERSION)
message(STATUS "[${PROJECT_NAME}] v${XFITSSFIX_SYS_VERSION}")

message(STATUS "[${PROJECT_NAME}] Size of a void pointer: ${CMAKE_SIZEOF_VOID_P}")

add_library(${PROJECT_NAME} STATIC "")
add_library(xfitssfix::sys ALIAS ${PROJECT_NAME})

set_target_properties(
	${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
		src
)

target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE
		_UNICODE
		UNICODE
		NOMINMAX
		WIN32_LEAN_AND_MEAN
		$<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
		$<$<CXX_COMPILER_ID:MSVC>:_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS>
)

target_compile_options(
	${PROJECT_NAME}
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:$<$<EQUAL:${CMAKE_SIZEOF_VOID_P},4>:-arch:IA32>>
		$<$<CXX_COMPILER_ID:MSVC>:-W4>
		$<$<CXX_COMPILER_ID:MSVC>:-MP -GR->
		$<$<CXX_COMPILER_ID:MSVC>:$<$<NOT:$<CONFIG:DEBUG>>:-Oy -GS->>
	PRIVATE
		$<$<CXX_COMPILER_ID:GNU>:$<$<EQUAL:${CMAKE_SIZEOF_VOID_P},4>:-march=i686>>
		$<$<CXX_COMPILER_ID:GNU>:-Wpedantic -Wall -Wextra>
)

target_link_options(
	${PROJECT_NAME}
	PRIVATE
		$<$<CXX_COMPILER_ID:GNU>:-s -static>
)

target_sources(
	${PROJECT_NAME}
	PRIVATE
		include/xfitssfix/c_string.h
		include/xfitssfix/core_types.h
		include/xfitssfix/condition_variable.h
		include/xfitssfix/encoding.h
		include/xfitssfix/env.h
		include/xfitssfix/exception.h
		include/xfitssfix/file.h
		include/xfitssfix/fs.h
		include/xfitssfix/fs_path.h
		include/xfitssfix/logger.h
		include/xfitssfix/moveable_mutex_lock.h
		include/xfitssfix/mutex.h
		include/xfitssfix/platform.h
		include/xfitssfix/process.h
		include/xfitssfix/scope_exit.h
		include/xfitssfix/shared_library.h
		include/xfitssfix/span.h
		include/xfitssfix/string.h
		include/xfitssfix/system_time.h
		include/xfitssfix/thread.h
		include/xfitssfix/rc.h
	PRIVATE
		src/xfitssfix/sys_win32_critical_section.h
		src/xfitssfix/sys_win32_condition_variable.h
		src/xfitssfix/sys_win32_event.h
)

target_sources(
	${PROJECT_NAME}
	PRIVATE
		src/exception.cpp
		src/fs.cpp
		src/fs_path.cpp
		src/logger.cpp
		src/moveable_mutex_lock.cpp
		src/string.cpp
		src/system_time.cpp
	PRIVATE
		src/win32_condition_variable.cpp
		src/win32_encoding.cpp
		src/win32_env.cpp
		src/win32_file.cpp
		src/win32_mutex.cpp
		src/win32_platform.cpp
		src/win32_process.cpp
		src/win32_shared_library.cpp
		src/win32_string.cpp
		src/win32_system_time.cpp
		src/win32_thread.cpp
	PRIVATE
		src/sys_win32_critical_section.cpp
		src/sys_win32_condition_variable.cpp
		src/sys_win32_event.cpp
)
