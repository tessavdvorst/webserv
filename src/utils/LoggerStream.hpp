#ifndef LOGGERSTREAM_HPP
# define LOGGERSTREAM_HPP

# include <sstream>
# include "Logger.hpp"

class LoggerStream {
    private:
        friend class Logger;
        LoggerStream(Logger& logger, Logger::LogLevel level);
		LoggerStream(const LoggerStream& that);

        Logger& logger;
        Logger::LogLevel level;
        std::ostringstream stream;
    
	public:
		~LoggerStream();

        template<typename T>
        LoggerStream& operator<<(const T& value) {
            stream << value;
            return (*this);
        }

};

#endif
