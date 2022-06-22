// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <cassert>
#include <xfitssfix/c_string.h>
#include <xfitssfix/exception.h>

namespace xfitssfix {

Exception::Exception(const char* message) noexcept
	:
	Exception{nullptr, message}
{}

Exception::Exception(const char* context, const char* message) noexcept
{
	const auto context_size = (context != nullptr ? c_string::get_size(context) : 0);
	const auto has_contex = (context_size > 0);

	const auto message_size = (message != nullptr ? c_string::get_size(message) : 0);
	const auto has_message = (message_size > 0);

	if (!has_contex && !has_message)
	{
		return;
	}

	constexpr auto left_prefix = "[";
	constexpr auto left_prefix_size = c_string::get_size(left_prefix);

	constexpr auto right_prefix = "] ";
	constexpr auto right_prefix_size = c_string::get_size(right_prefix);

	const auto what_size =
		(
			has_contex ?
			left_prefix_size + context_size + right_prefix_size :
			0
		) +
		message_size +
		1;

	what_.reset(new (std::nothrow) char[what_size]);

	if (what_ == nullptr)
	{
		return;
	}

	auto what = what_.get();

	if (has_contex)
	{
		what = std::uninitialized_copy_n(left_prefix, left_prefix_size, what);
		what = std::uninitialized_copy_n(context, context_size, what);
		what = std::uninitialized_copy_n(right_prefix, right_prefix_size, what);
	}

	if (has_message)
	{
		what = std::uninitialized_copy_n(message, message_size, what);
	}

	*what = '\0';
}

Exception::Exception(const Exception& rhs) noexcept
{
	if (rhs.what_ == nullptr)
	{
		return;
	}

	const auto rhs_what = rhs.what_.get();
	const auto what_size = c_string::get_size(rhs_what);

	what_.reset(new (std::nothrow) char[what_size + 1]);

	if (what_ == nullptr)
	{
		return;
	}

	auto what = what_.get();
	what = std::uninitialized_copy_n(rhs_what, what_size, what);
	*what = '\0';
}

const char* Exception::what() const noexcept
{
	return what_ != nullptr ? what_.get() : "[XFITSSFIX_EXCEPTION] Generic failure.";
}

} // namespace xfitssfix
