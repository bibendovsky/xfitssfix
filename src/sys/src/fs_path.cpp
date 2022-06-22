// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <xfitssfix/platform.h>
#include <cassert>
#include <algorithm>
#include <xfitssfix/fs_path.h>
#include <xfitssfix/c_string.h>
#include <xfitssfix/string.h>

namespace xfitssfix::fs {

Path::Path() = default;

Path::Path(const char* rhs_path, Int rhs_size)
{
	assert(rhs_path != nullptr);
	assert(rhs_size >= 0);
	assign(rhs_path, rhs_size);
}

Path::Path(const char* rhs_path)
{
	assert(rhs_path != nullptr);
	assign(rhs_path);
}

Path::Path(const Path& rhs_path)
	:
	data_{rhs_path.data_}
{}

Path::Path(Path&& rhs_path) noexcept
	:
	data_{std::move(rhs_path.data_)}
{}

Path& Path::operator=(const char* rhs_path)
{
	assert(rhs_path != nullptr);
	return assign(rhs_path);
}

Path& Path::operator=(const Path& rhs_path)
{
	if (this != std::addressof(rhs_path))
	{
		data_ = rhs_path.data_;
	}

	return *this;
}

Path& Path::operator=(Path&& rhs_path) noexcept
{
	data_ = std::move(rhs_path.data_);
	return *this;
}

bool Path::is_empty() const noexcept
{
	return data_.empty();
}

const char* Path::get_data() const noexcept
{
	return data_.data();
}

Int Path::get_size() const noexcept
{
	return static_cast<Int>(data_.size());
}

Path& Path::assign(const char* rhs_path, Int rhs_size)
{
	assert(rhs_path != nullptr);
	assert(rhs_size >= 0);
	data_.assign(rhs_path, rhs_size);
	normalize_separators(0);
	return *this;
}

Path& Path::assign(const char* rhs_path)
{
	assert(rhs_path != nullptr);
	const auto rhs_size = c_string::get_size(rhs_path);
	return assign(rhs_path, rhs_size);
}

Path& Path::assign(const Path& rhs_path)
{
	return assign(rhs_path.get_data(), rhs_path.get_size());
}

Path& Path::append(const char* rhs_path, Int rhs_size)
{
	assert(rhs_path != nullptr);
	assert(rhs_size >= 0);

	if (rhs_size == 0)
	{
		return *this;
	}

	if (is_empty())
	{
		return assign(rhs_path, rhs_size);
	}

	add_capacity(rhs_size + 1);


	const auto has_left_separator = (data_.back() == native_separator);

	const auto has_right_separator =
#ifdef XFITSSFIX_WIN32
		rhs_path[0] == '/' || rhs_path[0] == '\\'
#endif // XFITSSFIX_WIN32
		;

	if (has_left_separator && has_right_separator)
	{
		rhs_path += 1;
		rhs_size -= 1;
	}
	else if (!has_left_separator && !has_right_separator)
	{
		data_.push_back(native_separator);
	}

	const auto old_size = get_size();
	data_.append(rhs_path, rhs_size);
	normalize_separators(old_size);
	return *this;
}

Path& Path::append(const char* rhs_path)
{
	assert(rhs_path != nullptr);
	const auto rhs_size = c_string::get_size(rhs_path);
	return append(rhs_path, rhs_size);
}

Path& Path::append(const Path& rhs_path)
{
	return append(rhs_path.get_data(), rhs_path.get_size());
}

void Path::clear()
{
	data_.clear();
}

Path::Path(CapacityTag, Int capacity)
{
	assert(capacity >= 0);
	set_capacity(capacity);
}

Path::Path(SizeTag, Int size)
{
	assert(size >= 0);
	resize(size);
}

void Path::normalize_separators(Int offset)
{
#ifdef XFITSSFIX_WIN32
	std::replace(data_.begin() + offset, data_.end(), '/', '\\');
#endif // XFITSSFIX_WIN32
}

void Path::set_capacity(Int size)
{
	data_.reserve(static_cast<Data::size_type>(size));
}

void Path::add_capacity(Int size)
{
	set_capacity(get_size() + size);
}

void Path::resize(Int size)
{
	data_.resize(size);
}

// --------------------------------------------------------------------------

Path& operator/=(Path& lhs_path, const char* rhs_path)
{
	assert(rhs_path != nullptr);
	return lhs_path.append(rhs_path);
}

Path& operator/=(Path& lhs_path, const Path& rhs_path)
{
	return lhs_path.append(rhs_path);
}

Path operator/(const char* lhs_path, const Path& rhs_path)
{
	assert(lhs_path != nullptr);

	const auto lhs_size = c_string::get_size(lhs_path);
	const auto max_size = lhs_size + rhs_path.get_size() + 1;
	auto result = Path{Path::CapacityTag{}, max_size};
	result.assign(lhs_path, lhs_size);
	result.append(rhs_path);
	return result;
}

Path operator/(const Path& lhs, const char* rhs)
{
	assert(rhs != nullptr);
	const auto rhs_size = c_string::get_size(rhs);
	const auto max_size = lhs.get_size() + rhs_size + 1;
	auto result = Path{Path::CapacityTag{}, max_size};
	result.assign(lhs);
	result.append(rhs, rhs_size);
	return result;
}

Path operator/(const Path& lhs_path, const Path& rhs_path)
{
	const auto max_size = lhs_path.get_size() + rhs_path.get_size() + 1;
	auto result = Path{Path::CapacityTag{}, max_size};
	result.assign(lhs_path);
	result.append(rhs_path);
	return result;
}

} // namespace xfitssfix::fs
