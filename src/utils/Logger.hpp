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

		static LoggerStream log(const LogLevel log);
		static void print(const LogLevel log, const std::string msg);
		static std::string LevelToString(const LogLevel level);
		static std::string getColorCode(const LogLevel level);
};

#endif
