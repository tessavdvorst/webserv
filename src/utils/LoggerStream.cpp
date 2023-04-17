#include "LoggerStream.hpp"

// ============================= CONSTRUCTOR ===================================

LoggerStream::LoggerStream(Logger::LogLevel level): _level(level), _stream() {}

LoggerStream::LoggerStream(const LoggerStream& that): _level(that._level), _stream(that._stream.str()) {}

// ============================= DESTRUCTOR ===================================

LoggerStream::~LoggerStream() {
	const std::string message = this->_stream.str();
	if (!message.empty())
		Logger::print(this->_level, message);
}
