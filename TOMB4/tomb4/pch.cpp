#include "../tomb4/pch.h"
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

static std::shared_ptr<spdlog::logger> log_file = spdlog::basic_logger_mt("file", "logs/debug.txt", true);
static std::shared_ptr<spdlog::logger> log_console = spdlog::stdout_color_mt("console");

void SetupLogger()
{
	log_file->set_level(spdlog::level::trace);
	log_console->set_level(spdlog::level::trace);
}

void Log(LogType type, LPCSTR message, ...)
{
	char buffer[512] = {};
	static char logTypeErrorMsg[] = "Failed to log a message, LogType is unknown !";
	va_list args;
	va_start(args, message);
	vsprintf_s(buffer, message, args);
	va_end(args);

	switch (type)
	{
	case LogType::Debug:
		log_file->debug(buffer);
		log_console->debug(buffer);
		break;
	case LogType::Info:
		log_file->info(buffer);
		log_console->info(buffer);
		break;
	case LogType::Warn:
		log_file->warn(buffer);
		log_console->warn(buffer);
		break;
	case LogType::Error:
		log_file->error(buffer);
		log_console->error(buffer);
		break;
	case LogType::Fatal:
		log_file->critical(buffer);
		log_console->critical(buffer);
		MessageBox(NULL, buffer, "Tomb Raider IV", MB_OK | MB_ICONERROR | MB_TOPMOST);
		exit(EXIT_FAILURE);
		break;
	default:
		log_file->warn(logTypeErrorMsg);
		log_console->warn(logTypeErrorMsg);
		break;
	}
}
