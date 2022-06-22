// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_EXCEPTION_INCLUDED
#define XFITSSFIX_EXCEPTION_INCLUDED

#include <exception>
#include <memory>

namespace xfitssfix {

class Exception : public std::exception
{
public:
	explicit Exception(const char* message) noexcept;
	Exception(const char* context, const char* message) noexcept;
	Exception(const Exception& rhs) noexcept;

	const char* what() const noexcept override;

private:
	using What = std::unique_ptr<char[]>;

	What what_{};
};

} // namespace xfitssfix

#endif // !XFITSSFIX_EXCEPTION_INCLUDED
