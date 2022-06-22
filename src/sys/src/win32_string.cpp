// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#define STRSAFE_NO_DEPRECATE

#include <type_traits>
#include <strsafe.h>
#include <xfitssfix/exception.h>
#include <xfitssfix/string.h>

namespace xfitssfix {

class ToStringException : public Exception
{
public:
	explicit ToStringException(const char* message)
		:
		Exception{"WIN32_TO_STRING", message}
	{}
};

// --------------------------------------------------------------------------

namespace {

template<typename T>
constexpr int to_string_max_length() noexcept
{
	constexpr auto type_size = sizeof(T);

	if constexpr (std::is_integral_v<T>)
	{
		if constexpr (type_size == 4)
		{
			return std::is_signed_v<T> + 10;
		}

		if constexpr (type_size == 8)
		{
			return 20;
		}
	}

	if constexpr (std::is_floating_point_v<T>)
	{
		if constexpr (type_size == 4)
		{
			return 16;
		}
	}

	if constexpr (std::is_pointer_v<T>)
	{
		return 2 * static_cast<int>(sizeof(void*));
	}

	return 0;
}


template<typename T>
inline void to_string_generic(T value, const char* format_string, String& string)
{
	constexpr auto max_length = to_string_max_length<T>();
	string.resize(max_length);
	const auto data = string.data();
	char* data_end = nullptr;

	const auto win32_result = StringCbPrintfExA(
		data,
		max_length + 1,
		&data_end,
		nullptr,
		0,
		format_string,
		value);

	if (FAILED(win32_result))
	{
		throw ToStringException{"Failed to convert."};
	}

	string.resize(static_cast<String::size_type>(data_end - data));
}

} // namespace

// --------------------------------------------------------------------------

void to_string(int value, String& string)
{
	to_string_generic(value, "%d", string);
}

void to_string(unsigned int value, String& string)
{
	to_string_generic(value, "%u", string);
}

void to_string(long long value, String& string)
{
	to_string_generic(value, "%lld", string);
}

void to_string(unsigned long long value, String& string)
{
	to_string_generic(value, "%llu", string);
}

void to_string(float value, String& string)
{
	to_string_generic(value, "%f", string);
}

// --------------------------------------------------------------------------

String to_string(int value)
{
	static_assert(sizeof(decltype(value)) == 4);
	auto string = String{};
	to_string_generic(value, "%d", string);
	return string;
}

String to_string(unsigned int value)
{
	static_assert(sizeof(decltype(value)) == 4);
	auto string = String{};
	to_string_generic(value, "%u", string);
	return string;
}

String to_string(long value)
{
	static_assert(sizeof(decltype(value)) == 4);
	return to_string(static_cast<int>(value));
}

String to_string(unsigned long value)
{
	static_assert(sizeof(decltype(value)) == 4);
	return to_string(static_cast<unsigned int>(value));
}

String to_string(long long value)
{
	static_assert(sizeof(decltype(value)) == 8);
	auto string = String{};
	to_string_generic(value, "%lld", string);
	return string;
}

String to_string(unsigned long long value)
{
	static_assert(sizeof(decltype(value)) == 8);
	auto string = String{};
	to_string_generic(value, "%llu", string);
	return string;
}

String to_string(float value)
{
	static_assert(sizeof(float) == 4);
	auto string = String{};
	to_string_generic(value, "%f", string);
	return string;
}

String to_string_hex(const void* value)
{
	auto string = String{};
	to_string_generic(value, "%p", string);
	return string;
}

} // namespace xfitssfix
