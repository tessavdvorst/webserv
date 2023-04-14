#include "LoggerStream.hpp"

// ============================= CONSTRUCTOR ===================================

LoggerStream::LoggerStream(Logger& logger, Logger::LogLevel level) : logger(logger), level(level), stream() {}

LoggerStream::LoggerStream(const LoggerStream& that): logger(that.logger), level(that.level), stream(that.stream.str()) {}

// ============================= DESTRUCTOR ===================================

LoggerStream::~LoggerStream() {
	const std::string message = stream.str();
	if (!message.empty())
		logger.print(level, message);
}
