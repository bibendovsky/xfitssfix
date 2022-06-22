// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <windows.h>
#include <xfitssfix/encoding.h>
#include <xfitssfix/exception.h>
#include <xfitssfix/file.h>

namespace xfitssfix {

class Win32FileException : public Exception
{
public:
	explicit Win32FileException(const char* message)
		:
		Exception{"WIN32_FILE", message}
	{}
};

// --------------------------------------------------------------------------

class Win32File : public File
{
public:
	Win32File(const char* path, FileOpenMode open_mode);
	~Win32File() override;

	void set_position(int position) override;
	void move_to_the_end() override;

	int read(void* buffer, int size) override;
	int write(const void* buffer, int size) override;

	void flush() override;

private:
	bool is_readable_{};
	bool is_writable_{};
	HANDLE handle_{};

	bool is_handle_valid() noexcept;
	void ensure_is_open();
};

// --------------------------------------------------------------------------

Win32File::Win32File(const char* path, FileOpenMode open_mode)
{
	if (path == nullptr || path[0] == '\0')
	{
		throw Win32FileException{"Null or empty path."};
	}

	constexpr auto mode_error_message = "Unsupported open mode.";

	const auto is_readable = (open_mode & file_open_mode_read) != 0;
	const auto is_writable = (open_mode & file_open_mode_write) != 0;
	const auto is_truncate = (open_mode & file_open_mode_truncate) != 0;

	auto win32_desired_access = DWORD{};

	if (is_readable)
	{
		win32_desired_access |= GENERIC_READ;
	}

	if (is_writable)
	{
		win32_desired_access |= GENERIC_WRITE;
	}

	if (win32_desired_access == 0)
	{
		throw Win32FileException{mode_error_message};
	}

	constexpr auto win32_share_mode = DWORD{FILE_SHARE_READ};

	auto win32_creation_disposition = DWORD{};

	if (is_truncate)
	{
		if (!is_writable)
		{
			throw Win32FileException{mode_error_message};
		}

		win32_creation_disposition = CREATE_ALWAYS;
	}
	else
	{
		if (is_readable)
		{
			win32_creation_disposition = OPEN_EXISTING;
		}
		else if (is_writable)
		{
			win32_creation_disposition = OPEN_ALWAYS;
		}
		else
		{
			throw Win32FileException{mode_error_message};
		}
	}

	const auto utf16_path = encoding::to_utf16(path);

	handle_ = CreateFileW(
		reinterpret_cast<LPCWSTR>(utf16_path.c_str()),
		win32_desired_access,
		win32_share_mode,
		nullptr,
		win32_creation_disposition,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);

	if (!is_handle_valid())
	{
		throw Win32FileException{"Failed to open file."};
	}

	is_readable_ = is_readable;
	is_writable_ = is_writable;
}

Win32File::~Win32File()
{
	if (is_handle_valid())
	{
		CloseHandle(handle_);
	}
}

void Win32File::set_position(int position)
{
	ensure_is_open();
	LARGE_INTEGER win32_position;
	win32_position.QuadPart = position;

	const auto win32_result = SetFilePointerEx(
		handle_,
		win32_position,
		nullptr,
		FILE_BEGIN);

	if (win32_result == FALSE)
	{
		throw Win32FileException{"Failed to set position."};
	}
}

void Win32File::move_to_the_end()
{
	ensure_is_open();
	auto win32_position = LARGE_INTEGER{};

	const auto win32_result = SetFilePointerEx(
		handle_,
		win32_position,
		nullptr,
		FILE_END);

	if (win32_result == FALSE)
	{
		throw Win32FileException{"Failed to set position to the end."};
	}
}

int Win32File::read(void* buffer, int size)
{
	ensure_is_open();

	if (!is_readable_)
	{
		throw Win32FileException{"Not readable."};
	}

	if (buffer == nullptr || size <= 0)
	{
		return 0;
	}

	DWORD read_size;

	const auto win32_result = ReadFile(
		handle_,
		buffer,
		static_cast<DWORD>(size),
		&read_size,
		nullptr);

	if (win32_result == FALSE)
	{
		throw Win32FileException{"I/O read error."};
	}

	return static_cast<int>(read_size);
}

int Win32File::write(const void* buffer, int size)
{
	ensure_is_open();

	if (!is_writable_)
	{
		throw Win32FileException{"Not writable."};
	}

	if (buffer == nullptr || size <= 0)
	{
		return 0;
	}

	DWORD written_size;

	const auto win32_result = WriteFile(
		handle_,
		buffer,
		static_cast<DWORD>(size),
		&written_size,
		nullptr);

	if (win32_result == FALSE)
	{
		throw Win32FileException{"I/O write error."};
	}

	return static_cast<int>(written_size);
}

void Win32File::flush()
{
	ensure_is_open();
	const auto win32_result = FlushFileBuffers(handle_);

	if (win32_result == FALSE)
	{
		throw Win32FileException{"Failed to flush."};
	}
}

bool Win32File::is_handle_valid() noexcept
{
	return handle_ && handle_ != INVALID_HANDLE_VALUE;
}

void Win32File::ensure_is_open()
{
	if (!is_handle_valid())
	{
		throw Win32FileException{"Not open."};
	}
}

// --------------------------------------------------------------------------

FileUPtr make_file(const char* path, FileOpenMode open_mode)
{
	return std::make_unique<Win32File>(path, open_mode);
}

} // namespace xfitssfix
