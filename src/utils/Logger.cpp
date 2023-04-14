#include "Logger.hpp"
#include "LoggerStream.hpp"

// ============================= CONSTRUCTOR ===================================

Logger::Logger() {}

// ============================= DESTRUCTOR ====================================

Logger::~Logger() {}

// ============================== LOGGING ======================================

LoggerStream Logger::log(const LogLevel log) { return (LoggerStream(*this, log)); }

void Logger::print(const LogLevel log, const std::string msg)
{
	if (log == ERROR)
		std::cerr << '[' << getColorCode(log) << LevelToString(log) << "\033[0m] " << msg << '\n';
	else
		std::cout << '[' << getColorCode(log) << LevelToString(log) << "\033[0m] " << msg << '\n';
}

std::string Logger::LevelToString(const LogLevel level)
{
	switch (level) {
		case INFO:
			return ("INFO");
		case WARN:
			return ("WARN");
		case DEBUG:
			return ("DEBUG");
		case ERROR:
			return ("ERROR");
		default:
			return ("");
	}
}

std::string Logger::getColorCode(const LogLevel level)
{
		switch (level) {
		case INFO:
			return ("\033[36m");
		case WARN:
			return ("\033[33m");
		case DEBUG:
			return ("\033[32m");
		case ERROR:
			return ("\033[31m");
		default:
			return ("\033[0m");
	}
}
