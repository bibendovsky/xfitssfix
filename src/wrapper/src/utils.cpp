// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <cassert>
#include <exception>
#include <xfitssfix/common_strings.h>
#include <xfitssfix/utils.h>

namespace xfitssfix::utils {

namespace {

struct ErrorMessages
{
	static constexpr auto null_current_exception = "Null current exception.";
	static constexpr auto generic_exception = "Generic exception.";
};

} // namespace

// ==========================================================================

void log_exception(Logger& logger, const char* message) noexcept
{
	const auto exception_ptr = std::current_exception();
	logger.error(common::Strings::empty);
	logger.error(common::Strings::less_than_signs_8);

	if (message != nullptr)
	{
		logger.error(message);
	}

	if (exception_ptr != nullptr)
	{
		try
		{
			std::rethrow_exception(exception_ptr);
		}
		catch (const std::exception& ex)
		{
			logger.error(ex.what());
		}
		catch (...)
		{
			logger.error(ErrorMessages::generic_exception);
		}
	}
	else
	{
		logger.error(ErrorMessages::null_current_exception);
	}

	logger.error(common::Strings::greater_than_signs_8);
	logger.error(common::Strings::empty);
}

} // namespace xfitssfix::utils
