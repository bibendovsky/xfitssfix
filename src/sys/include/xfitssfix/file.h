// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_FILE_INCLUDED
#define XFITSSFIX_FILE_INCLUDED

#include <memory>

namespace xfitssfix {

class File
{
public:
	File() = default;
	virtual ~File() = default;

	virtual void set_position(int position) = 0;
	virtual void move_to_the_end() = 0;

	virtual int read(void* buffer, int size) = 0;
	virtual int write(const void* buffer, int size) = 0;

	virtual void flush() = 0;
};

// --------------------------------------------------------------------------

enum FileOpenMode : unsigned int
{
	file_open_mode_none = 0,

	file_open_mode_read = 1 << 0,
	file_open_mode_write = 1 << 1,
	file_open_mode_truncate = 1 << 2,

	file_open_mode_read_write = file_open_mode_read | file_open_mode_write,
};

// --------------------------------------------------------------------------

using FileUPtr = std::unique_ptr<File>;

FileUPtr make_file(const char* path, FileOpenMode open_mode);

} // namespace xfitssfix

#endif // !XFITSSFIX_FILE_INCLUDED
