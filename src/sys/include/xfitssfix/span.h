// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_SPAN_INCLUDED
#define XFITSSFIX_SPAN_INCLUDED

#include <cassert>
#include <cstddef>
#include <limits>

namespace xfitssfix {

inline constexpr std::size_t dynamic_extent = std::numeric_limits<std::size_t>::max();

// --------------------------------------------------------------------------

template<typename T, std::size_t TExtent = dynamic_extent>
class Span
{
public:
	using size_type = std::size_t;

	constexpr Span() noexcept = default;

	constexpr Span(T* items, size_type size) noexcept
		:
		items_{items},
		size_{size}
	{}

	constexpr T* data() const noexcept
	{
		return items_;
	}

	constexpr size_type size() const noexcept
	{
		return size_;
	}

	constexpr bool empty() const noexcept
	{
		return size() == 0;
	}

	constexpr T* begin() const noexcept
	{
		assert(data());
		return data();
	}

	constexpr T* end() const noexcept
	{
		return begin() + size();
	}

	constexpr T& operator[](size_type index) const noexcept
	{
		assert(index < size());
		return data()[index];
	}

private:
	T* items_{};
	size_type size_{};
};

// --------------------------------------------------------------------------

template<typename T>
Span<T> make_span(T* items, typename Span<T>::size_type size)
{
	return Span<T>{items, size};
}

} // namespace xfitssfix

#endif // !XFITSSFIX_SPAN_INCLUDED
