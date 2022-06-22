// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#ifndef XFITSSFIX_LOGGER_INCLUDED
#define XFITSSFIX_LOGGER_INCLUDED

#include <memory>

namespace xfitssfix {

enum class LoggerMessageType
{
	info,
	warning,
	error,
};

// --------------------------------------------------------------------------

struct LoggerParam
{
	const char* file_path{};
};

// --------------------------------------------------------------------------

class Logger
{
public:
	Logger() = default;
	virtual ~Logger() = default;

	virtual bool has_file() const noexcept = 0;
	virtual void flush() noexcept = 0;
	virtual void set_immediate_mode() noexcept = 0;

	virtual void write(LoggerMessageType message_type, const char* message) noexcept = 0;
	virtual void info(const char* message) noexcept;
	virtual void warning(const char* message) noexcept;
	virtual void error(const char* message) noexcept;
};

// --------------------------------------------------------------------------

using LoggerUPtr = std::unique_ptr<Logger>;

// --------------------------------------------------------------------------

class NullableLogger final : public Logger
{
public:
	void make(const LoggerParam& param);

	bool has_file() const noexcept override;
	void flush() noexcept override;
	void set_immediate_mode() noexcept override;

	void write(LoggerMessageType message_type, const char* message) noexcept override;
	void info(const char* message) noexcept override;
	void warning(const char* message) noexcept override;
	void error(const char* message) noexcept override;

private:
	LoggerUPtr logger_{};
};

// --------------------------------------------------------------------------

LoggerUPtr make_logger(const LoggerParam& param);

} // namespace xfitssfix

#endif // !XFITSSFIX_LOGGER_INCLUDED
