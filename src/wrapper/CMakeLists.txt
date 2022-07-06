# XFITSSFIX - X-Fi Titanium Streaming Sound Fix
# Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15.7 FATAL_ERROR)
project(xfitssfix VERSION 0.9.2 LANGUAGES CXX)

if (NOT WIN32)
	message(FATAL_ERROR "Unsupported platform.")
endif ()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(make_version_string)

make_version_string(XFITSSFIX_VERSION)
message(STATUS "[${PROJECT_NAME}] v${XFITSSFIX_VERSION}")

message(STATUS "[${PROJECT_NAME}] Size of a void pointer: ${CMAKE_SIZEOF_VOID_P}")

add_library(${PROJECT_NAME} SHARED "")

configure_file(
	"${PROJECT_SOURCE_DIR}/src/main.rc.in"
	"${PROJECT_BINARY_DIR}/src/main.rc"
)

set_target_properties(
	${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
		PREFIX ""
		OUTPUT_NAME OpenAL32
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		include
)

target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE
		"XFITSSFIX_VERSION=\"${XFITSSFIX_VERSION}\""
		_UNICODE
		UNICODE
		NOMINMAX
		WIN32_LEAN_AND_MEAN
		"AL_API=__declspec(dllexport)"
		"ALC_API=__declspec(dllexport)"
		AL_NO_PROTOTYPES
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

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		xfitssfix::sys
)

target_sources(
	${PROJECT_NAME}
	PRIVATE
		include/xfitssfix/al.h
		include/xfitssfix/alc.h
		include/xfitssfix/efx.h
	PRIVATE
		include/xfitssfix/al_api.h
		include/xfitssfix/al_api_utils.h
		include/xfitssfix/al_loader.h
		include/xfitssfix/al_object.h
		include/xfitssfix/al_symbols.h
	PRIVATE
		include/xfitssfix/common_strings.h
		include/xfitssfix/main.h
		include/xfitssfix/utils.h
)

target_sources(
	${PROJECT_NAME}
	PRIVATE
		src/main.cpp
		src/utils.cpp
	PRIVATE
		src/al_api.cpp
		src/al_api_utils.cpp
		src/al_loader.cpp
		src/al_object.cpp
	PRIVATE
		src/main.def
		"${PROJECT_BINARY_DIR}/src/main.rc"
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ".")
install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION "." OPTIONAL)
install(FILES ../../README.md DESTINATION "." RENAME "${PROJECT_NAME}_readme.txt")
install(FILES ../../LICENSE.md DESTINATION "." RENAME "${PROJECT_NAME}_license.txt")
install(FILES ../../CHANGELOG.md DESTINATION "." RENAME "${PROJECT_NAME}_changelog.txt")
