// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_SCOPE_EXIT_INCLUDED
#define XFITSSFIX_SCOPE_EXIT_INCLUDED

#include <utility>

namespace xfitssfix {

template<typename TFunctor>
class ScopeExit
{
public:
	ScopeExit() = delete;

	explicit ScopeExit(TFunctor&& functor) noexcept
		:
		functor_{std::move(functor)}
	{}

	ScopeExit(const ScopeExit& rhs) = delete;
	ScopeExit& operator=(const ScopeExit& rhs) = delete;

	~ScopeExit()
	{
		functor_();
	}

private:
	TFunctor functor_;
};

// --------------------------------------------------------------------------

template<typename TFunctor>
inline auto make_scope_exit(TFunctor&& functor) noexcept
{
	return ScopeExit<TFunctor>{std::forward<TFunctor>(functor)};
}

} // namespace xfitssfix

#endif // !XFITSSFIX_SCOPE_EXIT_INCLUDED
