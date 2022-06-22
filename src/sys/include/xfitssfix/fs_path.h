// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_FS_PATH_INCLUDED
#define XFITSSFIX_FS_PATH_INCLUDED

#include <string>
#include <xfitssfix/platform.h>
#include <xfitssfix/core_types.h>

namespace xfitssfix::fs {

constexpr auto native_separator =
#ifdef XFITSSFIX_WIN32
	'\\'
#endif // XFITSSFIX_WIN32
;

// --------------------------------------------------------------------------

class Path
{
public:
	Path();
	Path(const char* rhs_path, Int rhs_size);
	explicit Path(const char* rhs_path);
	Path(const Path& rhs_path);
	Path(Path&& rhs_path) noexcept;

	Path& operator=(const char* rhs_path);
	Path& operator=(const Path& rhs_path);
	Path& operator=(Path&& rhs_path) noexcept;

	bool is_empty() const noexcept;
	const char* get_data() const noexcept;
	Int get_size() const noexcept;

	Path& assign(const char* rhs_path, Int rhs_size);
	Path& assign(const char* rhs_path);
	Path& assign(const Path& rhs_path);

	Path& append(const char* rhs_path, Int rhs_size);
	Path& append(const char* rhs_path);
	Path& append(const Path& rhs_path);

	void clear();

private:
	struct CapacityTag {};
	struct SizeTag {};

	using Data = std::string;

	Data data_{};

	friend Path operator/(const char* lhs_path, const Path& rhs_path);
	friend Path operator/(const Path& lhs_path, const char* rhs_path);
	friend Path operator/(const Path& lhs_path, const Path& rhs_path);

	Path(CapacityTag, Int capacity);
	Path(SizeTag, Int size);

	void normalize_separators(Int offset);

	void set_capacity(Int size);
	void add_capacity(Int size);

	void resize(Int size);
}; // Path

// ==========================================================================

Path& operator/=(Path& lhs, const char* rhs_path);
Path& operator/=(Path& lhs, const Path& rhs_path);

Path operator/(const char* lhs_path, const Path& rhs_path);
Path operator/(const Path& lhs_path, const char* rhs_path);
Path operator/(const Path& lhs_path, const Path& rhs_path);

} // namespace xfitssfix::fs

#endif // !XFITSSFIX_FS_PATH_INCLUDED
