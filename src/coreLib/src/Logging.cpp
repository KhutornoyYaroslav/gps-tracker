#include <memory>

#include <Logging.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#define LOGGER_LOG_DEFAULT_FORMAT "%Y-%m-%d %T.%e [%^%L%$]: %v"
#define LOGGER_LOG_ROTATTION_FILE_SIZE_MB 2
#define LOGGER_LOG_ROTATTION_FILE_COUNT 10

#define NEW_LINE SPDLOG_EOL

namespace logging {

	bool initLogger(const char *filePath, std::size_t fileSize, std::size_t filesCount)
	{
		auto console_sink_ = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink_->set_level(spdlog::level::trace);
		console_sink_->set_pattern(LOGGER_LOG_DEFAULT_FORMAT);

		auto file_sink_ = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filePath, fileSize, filesCount);
		file_sink_->set_level(spdlog::level::debug);
		file_sink_->set_pattern(LOGGER_LOG_DEFAULT_FORMAT);

		{
			auto logger = std::make_shared<spdlog::logger>("default", spdlog::sinks_init_list{ console_sink_, file_sink_ });
			//        logger->set_level(spdlog::level::trace);
			spdlog::set_default_logger(logger);
			spdlog::flush_every(std::chrono::seconds(3));
			spdlog::flush_on(spdlog::level::critical);
		}

		return true;
	}

	void shutdown()
	{
		spdlog::shutdown();
	}
}
