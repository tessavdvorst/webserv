#include "utils.hpp"

bool check_extension(const std::string& filename, const std::string& extension)
{
	if (filename.length() <= extension.length())
		return (false);
	if (filename.compare(filename.length() - extension.length(), extension.length(), extension) == 0)
		return (true);
	return (false);
}

void erase_whitespace_chars_both_sides(std::string& str)
{
	str.erase(0, str.find_first_not_of(" \t\r\n\v\f"));
	str.erase(str.find_last_not_of(" \t\r\n\v\f") + 1);
}

bool is_comment(std::string line)
{
	erase_whitespace_chars_both_sides(line);
	if (!line.empty() && line[0] == '#')
		return (true);
	return (false); 
}

int find_closing_bracket(std::vector<std::string> strVec, int index)
{
	int nestLevel = 0;
	while (index < static_cast<int>(strVec.size()))
	{
		size_t pos = strVec[index].find_first_of("{");
		if ((pos != std::string::npos)
				&& (strVec[index].substr(0, strlen("location")) == "location"))
			nestLevel++;
		else if (pos != std::string::npos)
			return (config_error_and_return("unknown block", index, -1));
		else if (!nestLevel && strVec[index] == "}")
			return (index);
		else if (nestLevel > 0 && strVec[index] == "}")
			nestLevel--;
		index++;
	}
	return (config_error_and_return("invalid syntax (brackets)", index - 1, -1));
}

void error_and_close(std::string errorMsg, int socket)
{
	if (socket >= 0)
		close(socket);
	Logger::log(Logger::ERROR) << errorMsg << '\n';
}
