// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <charconv>
#include <xfitssfix/system_time.h>

namespace xfitssfix {

void make_system_time_string(const SystemTime& system_time, String& system_time_string)
{
	// YYYY-MM-DD HH:mm:ss.sss
	constexpr auto max_string_length = 23;

	system_time_string.resize(max_string_length);

	const auto number_to_string = [](int number, int max_digit_count, char*& chars)
	{
		const auto end_chars = chars + max_digit_count;

		int digit_count;

		if (number > 999)
		{
			digit_count = 4;
		}
		else if (number > 99)
		{
			digit_count = 3;
		}
		else if (number > 9)
		{
			digit_count = 2;
		}
		else
		{
			digit_count = 1;
		}

		const auto zero_count = max_digit_count - digit_count;

		for (auto i = 0; i < zero_count; ++i)
		{
			*chars++ = '0';
		}

		std::to_chars(chars, end_chars, number);

		chars = end_chars;
	};

	auto chars = system_time_string.data();
	number_to_string(system_time.year, 4, chars);
	*chars++ = '-';
	number_to_string(system_time.month, 2, chars);
	*chars++ = '-';
	number_to_string(system_time.day, 2, chars);
	*chars++ = ' ';
	number_to_string(system_time.hour, 2, chars);
	*chars++ = ':';
	number_to_string(system_time.minute, 2, chars);
	*chars++ = ':';
	number_to_string(system_time.second, 2, chars);
	*chars++ = '.';
	number_to_string(system_time.millisecond, 3, chars);
	system_time_string.resize(chars - system_time_string.data());
}

} // namespace xfitssfix
