#ifndef LOGGER_HPP
# define LOGGER_HPP

# include <iostream>
# include <string>

class LoggerStream;

class Logger {
	public:
		Logger();
		~Logger();

		enum LogLevel {
			INFO,
			WARN,
			DEBUG,
			ERROR
		};

		LoggerStream log(const LogLevel log);
		void print(const LogLevel log, const std::string msg);
		std::string LevelToString(const LogLevel level);
		std::string getColorCode(const LogLevel level);
};

#endif
