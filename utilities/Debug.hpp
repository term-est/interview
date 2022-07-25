#pragma once

#include <source_location>
#include <optional>
#include <string>
#include <filesystem>

#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/chrono.h>
#include <spdlog/spdlog.h>

struct error_helper
{
	const std::int8_t error_code = -1;
	const std::optional<std::string> error_message;
	const std::optional<std::filesystem::path> log_file;

	bool enable_colors = true;
};

template <auto Function = nullptr, auto Exit = std::exit>
struct error : error_helper
{

	constexpr explicit error(error_helper e, std::source_location src_lct = std::source_location::current()) : error_helper{std::move(e)}, src_lct{src_lct}
	{}

	[[noreturn]]
	~error()    noexcept(noexcept(Function()))
	requires requires{Function();}
	{
		if(log_file.has_value())
		{
			enable_colors = false;
			spdlog::error(debug_log() + '\n' + error_message.value_or(""));
		}

		else
			spdlog::error(debug_log() + '\n' + error_message.value_or(""));

		(void) Function();
		Exit(error_code);
	}

	[[noreturn]]
	~error() noexcept
	{
		if(log_file.has_value())
		{
			enable_colors = false;
			spdlog::error(debug_log() + '\n' + error_message.value_or(""));
		}

		else
			spdlog::error(debug_log() + '\n' + error_message.value_or(""));

		Exit(error_code);
	}

private:

	inline auto debug_log() const
	{
		if(enable_colors)

			return " at function: " + fmt::format(fmt::fg(fmt::color::blue), "{:s}", src_lct.function_name())
				   + " in file: " + fmt::format(fmt::fg(fmt::color::blue), "{:s}", src_lct.file_name())
				   + " at line: " + fmt::format(fmt::fg(fmt::color::blue), "{:d}", src_lct.line());

		else
			return " at function: " + fmt::format("{:s}", src_lct.function_name())
				   + " in file: " + fmt::format("{:s}", src_lct.file_name())
				   + " at line: " + fmt::format("{:d}", src_lct.line());
	}

	std::source_location src_lct;
};

template <auto Function = nullptr, auto Exit = std::exit>
error(error_helper e, std::source_location src_lct = std::source_location::current()) -> error<nullptr, std::exit>;
