// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_AL_OBJECT_INCLUDED
#define XFITSSFIX_AL_OBJECT_INCLUDED

#include <algorithm>
#include <memory>
#include <xfitssfix/al.h>
#include <xfitssfix/al_symbols.h>

namespace xfitssfix {

template<typename TDeleter>
class AlObject
{
public:
	using Value = ALuint;
	using Deleter = TDeleter;

	AlObject() noexcept = default;

	explicit AlObject(Value value) noexcept
		:
		value_{value}
	{}

	AlObject(Value value, Deleter deleter) noexcept
		:
		value_{value},
		deleter_{deleter}
	{}

	AlObject(const AlObject& rhs) = delete;

	AlObject(AlObject&& rhs) noexcept
	{
		std::swap(value_, rhs.value_);
		std::swap(deleter_, rhs.deleter_);
	}

	AlObject& operator=(const AlObject& rhs) = delete;

	void operator=(AlObject&& rhs) noexcept
	{
		destroy();
		std::swap(value_, rhs.value_);
		std::swap(deleter_, rhs.deleter_);
	}

	~AlObject()
	{
		destroy();
	}

	bool has_value() const noexcept
	{
		return value_ != AL_NONE;
	}

	Value get() const noexcept
	{
		return value_;
	}

	void reset() noexcept
	{
		destroy();
	}

	void reset(Value value) noexcept
	{
		destroy();
		value_ = value;
	}

	Value release() noexcept
	{
		const auto al_name = value_;
		value_ = Value{};
		return al_name;
	}

private:
	Value value_{};
	Deleter deleter_{};

	void destroy() noexcept
	{
		if (value_ != AL_NONE)
		{
			const auto value = value_;
			value_ = AL_NONE;
			deleter_(value);
		}
	}
};

} // namespace xfitssfix

#endif // !XFITSSFIX_AL_OBJECT_INCLUDED
