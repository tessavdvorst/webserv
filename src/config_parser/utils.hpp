#ifndef UTILS_HPP
#define UTILS_HPP

# include <unistd.h>
# include <iostream>
# include <string>
# include <vector>
# include <cstring>
# include "LoggerStream.hpp"

extern Logger logger;

template <typename T>
T error_and_return(std::string errorMsg, T returnVal) {
	logger.log(Logger::ERROR) << errorMsg;
	return (returnVal);
}

template <typename T>
T config_error_and_return(std::string errorMsg, int index, T returnVal) {
	logger.log(Logger::ERROR) << errorMsg << " at line " << index + 1;
	return (returnVal);
}

bool		check_extension(const std::string& filename, const std::string& extension);
void		erase_whitespace_chars_both_sides(std::string& str);
int			find_closing_bracket(std::vector<std::string> strVec, int index);
bool		is_comment(std::string line);
void		error_and_close(std::string errorMsg, int socket);

#endif
