#ifndef UTILS_HPP
#define UTILS_HPP

# include <unistd.h>
# include <iostream>
# include <string>
# include <vector>
#include <cstring>
# include "LoggerStream.hpp"

extern Logger logger;

bool check_extension(const std::string& filename, const std::string& extension);
void erase_whitespace_chars_both_sides(std::string& str);
int find_closing_bracket(std::vector<std::string> strVec, int index);
bool is_comment(std::string line);

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

void error_and_close(std::string errorMsg, int socket);

// template <typename T>
// void container_deep_copy(T copy, T original)
// {
// 	copy.erase(copy.begin(), copy.end());
// 	copy.insert(copy.end(), original.begin(), original.end());
// }

// template <typename T>
// void print_vector(std::vector<T> vec) {
// 	typename std::vector<T>::iterator it;
// 	for (it = vec.begin(); it != vec.end(); it++)
// 		std::cout << *it << "\n";
// }

#endif
