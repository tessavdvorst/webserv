#ifndef LOGGERSTREAM_HPP
# define LOGGERSTREAM_HPP

# include <sstream>
# include "Logger.hpp"

class LoggerStream {
    private:
        friend class Logger;
        LoggerStream(Logger::LogLevel level);
		LoggerStream(const LoggerStream& that);

        Logger::LogLevel _level;
        std::ostringstream _stream;
    
	public:
		~LoggerStream();

        template<typename T>
        LoggerStream& operator<<(const T& value) {
            this->_stream << value;
            return (*this);
        }

};

#endif
