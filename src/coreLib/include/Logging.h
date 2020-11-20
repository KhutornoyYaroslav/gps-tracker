#pragma once

#define SPDLOG_ACTIVE_LEVEL 0
//#define SPDLOG_WCHAR_FILENAMES
#include <spdlog/spdlog.h>

#define LOGGER_LOG_ROTATTION_FILE_SIZE_MB 2
#define LOGGER_LOG_ROTATTION_FILE_COUNT 10

namespace logging
{
	extern bool initLogger(
		const char * filePath =
#if defined(_WIN32)
		"logs/log.txt",
#else
		"/var/log/duet/log.txt",
#endif
		std::size_t fileSize = LOGGER_LOG_ROTATTION_FILE_SIZE_MB * 1024 * 1024,
		std::size_t filesCount = LOGGER_LOG_ROTATTION_FILE_COUNT);

	extern void shutdown();

	static spdlog::level::level_enum strToLogLevel(const std::string &value)
	{
		if (value == "TRACE")     return spdlog::level::trace;
		if (value == "DEBUG")     return spdlog::level::debug;
		if (value == "INFO")      return spdlog::level::info;
		if (value == "WARNING")   return spdlog::level::warn;
		if (value == "ERROR")     return spdlog::level::err;
		if (value == "CRITICAL")  return spdlog::level::critical;
		if (value == "OFF")       return spdlog::level::off;
		return spdlog::level::info;
	}

	static const char* logLevelToStr(spdlog::level::level_enum value)
	{
		switch (value)
		{
		case spdlog::level::trace:      return "TRACE";
		case spdlog::level::debug:      return "DEBUG";
		case spdlog::level::info:       return "INFO";
		case spdlog::level::warn:       return "WARNING";
		case spdlog::level::err:        return "ERROR";
		case spdlog::level::critical:   return "CRITICAL";
		case spdlog::level::off:        return "OFF";
		default:                        return "INFO";
		}
	}
}

#define LOG_TRACE(...)     SPDLOG_TRACE(__VA_ARGS__)
#define LOG_DEBUG(...)     SPDLOG_DEBUG(__VA_ARGS__)
#define LOG_INFO(...)      SPDLOG_INFO(__VA_ARGS__)
#define LOG_WARNING(...)   SPDLOG_WARN(__VA_ARGS__)
#define LOG_ERROR(...)     SPDLOG_ERROR(__VA_ARGS__)
#define LOG_CRITICAL(...)  SPDLOG_CRITICAL(__VA_ARGS__)
