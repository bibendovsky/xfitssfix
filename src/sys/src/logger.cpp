// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <atomic>
#include <chrono>
#include <deque>
#include <exception>
#include <iterator>
#include <memory>
#include <mutex>
#include <xfitssfix/condition_variable.h>
#include <xfitssfix/file.h>
#include <xfitssfix/logger.h>
#include <xfitssfix/mutex.h>
#include <xfitssfix/process.h>
#include <xfitssfix/string.h>
#include <xfitssfix/system_time.h>
#include <xfitssfix/thread.h>

namespace xfitssfix {

void Logger::info(const char* message) noexcept
{
	write(LoggerMessageType::info, message);
}

void Logger::warning(const char* message) noexcept
{
	write(LoggerMessageType::warning, message);
}

void Logger::error(const char* message) noexcept
{
	write(LoggerMessageType::error, message);
}

// --------------------------------------------------------------------------

struct LoggerMessage
{
	LoggerMessageType type{};
	String message{};
	bool is_written{};

	LoggerMessage(LoggerMessageType type, const char* message)
		:
		type{type},
		message{message}
	{}
};

// --------------------------------------------------------------------------

class LoggerImpl final : public Logger
{
public:
	LoggerImpl(const LoggerParam& param);
	~LoggerImpl() override;

	bool has_file() const noexcept override;
	void flush() noexcept override;
	void set_immediate_mode() noexcept override;

	void write(LoggerMessageType message_type, const char* message) noexcept override;

private:
	static constexpr auto min_level = 0;
	static constexpr auto max_level = 32;

	using Messages = std::deque<LoggerMessage>;

	bool is_shared_library_{};
	FileUPtr file_{};
	String timestamp_buffer_{};
	String message_buffer_{};
	bool has_messages_{};
	bool is_flushing_{};
	bool is_flushing_ack_{};
	bool is_quit_thread_{};
	bool is_quit_thread_ack_{};
	Messages messages_{};
	Messages mt_messages_{};
	MutexUPtr mutex_{};
	ConditionVariable cv_{};
	ConditionVariable cv_ack_{};
	ThreadUPtr thread_{};

	static String make_timestamp_buffer();
	static String make_message_buffer();

	void write_messages(Messages& messages) noexcept;
	void write_messages() noexcept;

	static void thread_func_proxy(void* arg);
	void thread_func();

	void write_internal(const std::exception& ex);

	void set_immediate_mode_internal() noexcept;
};

// --------------------------------------------------------------------------

LoggerImpl::LoggerImpl(const LoggerParam& param)
{
	is_shared_library_ = process::is_shared_library();
	timestamp_buffer_ = make_timestamp_buffer();
	message_buffer_ = make_message_buffer();
	mutex_ = make_mutex();

	try
	{
		file_ = make_file(
			param.file_path,
			static_cast<FileOpenMode>(
				FileOpenMode::file_open_mode_read_write |
				FileOpenMode::file_open_mode_truncate));
	}
	catch (...)
	{
	}

	thread_ = make_thread(thread_func_proxy, this);
}

LoggerImpl::~LoggerImpl()
{
	set_immediate_mode_internal();
}

bool LoggerImpl::has_file() const noexcept
{
	return file_ != nullptr;
}

void LoggerImpl::flush() noexcept
{
	auto is_notify = false;

	{
		const auto lock = std::unique_lock{*mutex_};

		if (is_quit_thread_)
		{
			write_messages();
		}
		else
		{
			is_notify = true;
			is_flushing_ = true;
			is_flushing_ack_ = false;
		}
	}

	if (is_notify)
	{
		cv_.notify_one();
		auto lock = std::unique_lock{*mutex_};

		while (!is_flushing_ack_)
		{
			cv_ack_.wait(lock);
		}
	}
}

void LoggerImpl::set_immediate_mode() noexcept
{
	set_immediate_mode_internal();
}

void LoggerImpl::write(LoggerMessageType message_type, const char* message) noexcept
try
{
	const auto lock = std::unique_lock{*mutex_};
	messages_.emplace_back(LoggerMessage{message_type, message});

	if (is_quit_thread_)
	{
		write_messages();
	}
	else
	{
		has_messages_ = true;
		cv_.notify_one();
	}
}
catch (...)
{
}

String LoggerImpl::make_timestamp_buffer()
{
	auto message_buffer = String{};
	message_buffer.resize(23);
	return message_buffer;
}

String LoggerImpl::make_message_buffer()
{
	auto message_buffer = String{};
	message_buffer.reserve(2048);
	return message_buffer;
}

void LoggerImpl::write_messages(Messages& messages) noexcept
{
	if (messages.empty())
	{
		return;
	}

	if (file_ == nullptr)
	{
		messages.clear();
		return;
	}

	for (auto& message : messages)
	{
		if (message.is_written)
		{
			continue;
		}

		const auto& system_time = make_system_time();
		make_system_time_string(system_time, timestamp_buffer_);

		message_buffer_.clear();
		message_buffer_ += '[';
		message_buffer_ += timestamp_buffer_;
		message_buffer_ += "] ";
		message_buffer_ += "[XFITSSFIX] ";
		message_buffer_ += '[';

		switch (message.type)
		{
			case LoggerMessageType::info:
				message_buffer_ += 'I';
				break;

			case LoggerMessageType::warning:
				message_buffer_ += 'W';
				break;

			case LoggerMessageType::error:
				message_buffer_ += 'E';
				break;

			default:
				message_buffer_ += '?';
				break;
		}

		message_buffer_ += ']';

		if (!message.message.empty())
		{
			if (!message_buffer_.empty())
			{
				message_buffer_ += ' ';
			}

			message_buffer_ += message.message;
		}

		message_buffer_ += '\n';

		try
		{
			file_->write(message_buffer_.c_str(), static_cast<int>(message_buffer_.size()));
		}
		catch (...)
		{
		}

		message.is_written = true;
	}

	try
	{
		file_->flush();
	}
	catch (...)
	{
	}

	messages.clear();
}

void LoggerImpl::write_messages() noexcept
{
	write_messages(mt_messages_);
	write_messages(messages_);
}

void LoggerImpl::thread_func_proxy(void* arg)
{
	static_cast<LoggerImpl*>(arg)->thread_func();
}

void LoggerImpl::thread_func()
try
{
	thread::set_name("xfitssfix_logger");

	while (true)
	{
		auto is_flushing = false;

		{
			auto lock = std::unique_lock{*mutex_};

			while (!is_quit_thread_ && !is_flushing_ && !has_messages_)
			{
				cv_.wait(lock);
			}

			has_messages_ = false;

			if (is_quit_thread_)
			{
				break;
			}

			is_flushing = is_flushing_;

			mt_messages_.insert(
				mt_messages_.end(),
				std::make_move_iterator(messages_.begin()),
				std::make_move_iterator(messages_.end()));

			messages_.clear();
		}

		write_messages(mt_messages_);

		if (is_flushing)
		{
			const auto lock = std::unique_lock{*mutex_};
			is_flushing_ = false;
			is_flushing_ack_ = true;
			cv_ack_.notify_one();
		}
	}

	{
		auto lock = std::unique_lock{*mutex_};
		is_quit_thread_ack_ = true;
		cv_ack_.notify_one();
	}
}
catch (...)
{
}

void LoggerImpl::write_internal(const std::exception& ex)
{
	error(ex.what());
}

void LoggerImpl::set_immediate_mode_internal() noexcept
try
{
	auto is_notify = false;

	{
		const auto lock = std::unique_lock{*mutex_};

		if (!is_quit_thread_)
		{
			is_notify = true;
			is_quit_thread_ = true;
			cv_.notify_one();
		}
	}

	if (is_notify)
	{
		auto lock = std::unique_lock{*mutex_};

		if (is_shared_library_)
		{
			// If logger instance was created inside DLL we can't wait indefinitely.
			// Threads could be already terminated.

			constexpr auto wait_duration_ms = std::chrono::milliseconds{10};
			cv_ack_.wait_for(lock, wait_duration_ms);
		}
		else
		{
			while (!is_quit_thread_ack_)
			{
				cv_ack_.wait(lock);
			}
		}
	}

	write_messages();
}
catch (...)
{
}

// --------------------------------------------------------------------------

void NullableLogger::make(const LoggerParam& param)
{
	logger_ = nullptr;
	logger_ = make_logger(param);
}

bool NullableLogger::has_file() const noexcept
{
	if (logger_ != nullptr)
	{
		return logger_->has_file();
	}

	return false;
}

void NullableLogger::flush() noexcept
{
	if (logger_ != nullptr)
	{
		logger_->flush();
	}
}

void NullableLogger::set_immediate_mode() noexcept
{
	if (logger_ != nullptr)
	{
		logger_->set_immediate_mode();
	}
}

void NullableLogger::write(LoggerMessageType message_type, const char* message) noexcept
{
	if (logger_ != nullptr)
	{
		logger_->write(message_type, message);
	}
}

void NullableLogger::info(const char* message) noexcept
{
	if (logger_ != nullptr)
	{
		logger_->info(message);
	}
}

void NullableLogger::warning(const char* message) noexcept
{
	if (logger_ != nullptr)
	{
		logger_->warning(message);
	}
}

void NullableLogger::error(const char* message) noexcept
{
	if (logger_ != nullptr)
	{
		logger_->error(message);
	}
}

// --------------------------------------------------------------------------

LoggerUPtr make_logger(const LoggerParam& param)
{
	return std::make_unique<LoggerImpl>(param);
}

} // namespace xfitssfix
