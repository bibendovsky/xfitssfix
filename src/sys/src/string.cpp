// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <limits>
#include <string_view>
#include <xfitssfix/exception.h>
#include <xfitssfix/string.h>

namespace xfitssfix {

class StoiException : public Exception
{
public:
	explicit StoiException(const char* message)
		:
		Exception{"STOI", message}
	{}
};

// --------------------------------------------------------------------------

int stoi(String string)
{
	auto string_view = std::string_view{string.data(), string.size()};

	if (string_view.empty() || string_view.size() > 11)
	{
		throw StoiException{"String size out of range."};
	}

	auto is_negative = false;

	switch (string_view.front())
	{
		case '-':
			is_negative = true;
			string_view.remove_prefix(1);
			break;

		case '+':
			string_view.remove_prefix(1);
			break;

		default:
			break;
	}

	auto value = 0LL;

	for (const auto digit_char : string_view)
	{
		const auto digit = digit_char - '0';

		if (digit < 0 || digit > 9)
		{
			throw StoiException{"Invalid digit."};
		}

		value += (value * 10) + digit;
	}

	if (is_negative)
	{
		value = -value;
	}

	static constexpr auto min_value = std::numeric_limits<int>::min();
	static constexpr auto max_value = std::numeric_limits<int>::max();

	if (value < min_value || value > max_value)
	{
		throw StoiException{"Value out of range."};
	}

	return static_cast<int>(value);
}

} // namespace xfitssfix
