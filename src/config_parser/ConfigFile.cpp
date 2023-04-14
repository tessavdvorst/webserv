#include "ConfigFile.hpp"
#include "ServerBlock.hpp"

/*
								DOCUMENTATION
==============================================================================

-> a config file can contain multiple serverblocks
-> there may be multiple location blocks within a serverblock
-> a locationblock can only reside wihtin a serverblock
-> a locationblock cannot contain a servblock or another locationblock
-> each block should have opening and closing brackets
-> all directives should be ended with `;`
-> the `listen` directive requires a port number
-> each block should at least contain a `listen` and `server_name` directive
-> duplicate directives are not allowed for `listen`, `server_name` and `root`
-> For the other directives, the last occurence will be used

*/

// =========================== CONSTRUCTORS =================================

ConfigFile::ConfigFile() {}

// ConfigFile::ConfigFile(const ConfigFile& that) {}

ConfigFile::ConfigFile(const std::string& fileName): _fileName(fileName)
{
	load_file();
	save_configurations();
}

// ============================ DESTRUCTORS =================================

ConfigFile::~ConfigFile() {}

// ====================== ASSIGN OPERATOR OVERLOAD ============================

// ConfigFile& ConfigFile::operator=(const ConfigFile& that) {}

// ============================== PARSER ====================================

void ConfigFile::load_file(void)
{
	std::string line;
	std::ifstream file(this->_fileName.c_str());

	if (!file)
		throw InvalidFileException();
	while (getline(file, line))
	{
		erase_whitespace_chars_both_sides(line);
		this->_content.push_back(line);
	}
	file.close();
}

void ConfigFile::save_configurations(void)
{
	check_syntax_server_block();
	for (int i = 0; i < static_cast<int>(this->_content.size()); i++)
	{
		ServerBlock server;
		i = server.get_first_direc_pos(this->_content, i);
		if (i == static_cast<int>(this->_content.size()))
			return ;
		while (i < static_cast<int>(this->_content.size()))
		{
			while (i < static_cast<int>(this->_content.size())
				&& (this->_content[i].empty() || is_comment(this->_content[i])))
				i++;
			Block::ConfigType dType = server.get_direc_type(this->_content[i]);
			if (dType == Block::LISTEN_DIRECTIVE)
				server.save_port(this->_content[i], i);
			else if (dType == Block::SERVER_NAME_DIRECTIVE)
				server.save_server_name(this->_content[i], i);
			else if (dType == Block::DIRECTIVE)
				server.save_directive(this->_content[i], i);
			else if (dType == Block::LOCATION)
			{
				int start = i;
				i = find_closing_bracket(this->_content, i + 1);
				std::vector<std::string> singleLocatioBlock(this->_content.begin() + start, this->_content.end());
				server.save_location(singleLocatioBlock, start);
			}
			else if (dType == Block::END_OF_BLOCK)
				break;
			else
				throw InvalidSyntaxException(i, UNKNOWN_DIRECTIVE);
			i++;
		}
		this->_serverBlock.push_back(server);
	}
}

// =========================== SYNTAX CHECKS =================================

bool ConfigFile::is_valid_location(const std::string& context)
{
	std::string arg;
	std::stringstream ss(context);
	std::vector<std::string> args;

	while (std::getline(ss, arg, ' '))
		args.push_back(arg);
	if (args.size() == 3)
	{
		if (args[0] == "location"
			&& (args[1][0] == '/')
			&& args[2] == "{")
			return (true);
	}
	return (false);
}

bool ConfigFile::is_correct_end(const std::string& line, int index)
{
	size_t pos;
	std::string temp = line;
	pos = temp.find_first_of("#");
	if (pos != std::string::npos)
		temp.erase(pos, temp.size() - pos);

	erase_whitespace_chars_both_sides(temp);
	
	if (temp.back() != ';')
		return (false);
	if (std::count(temp.begin(), temp.end(), ';') > 1)
		logger.log(Logger::WARN) << "directive at line " << index + 1 << " contains multiple semicolons";
	return (true);
}

Block::ConfigType ConfigFile::check_syntax_line(const std::string& line, int index)
{
	size_t pos;
	pos = line.find_first_of("{}");
	if (pos == std::string::npos)
	{
		if (!is_correct_end(line, index))
			return (Block::UNKNOWN);
		if (line.substr(0, strlen("listen")) == "listen")
			return (Block::LISTEN_DIRECTIVE);
		if (line.substr(0, strlen("server_name")) == "server_name")
			return (Block::SERVER_NAME_DIRECTIVE);
		if (line.substr(0, strlen("client_max_body_size")) == "client_max_body_size"
				&& ((line.substr((line.length() - 2), 1)[0] == 'm' || line.substr((line.length() - 2), 1)[0] == 'M'
					|| line.substr((line.length() - 2), 1)[0] == 'k' || line.substr((line.length() - 2), 1)[0] == 'K')
					|| line.substr((line.length() - 2), 1)[0] == 'b' || !isdigit(line.substr((line.length() - 2), 1)[0])))
			return (Block::CLIENT_MAX_BODY);
		return (Block::DIRECTIVE);
	}
	if (line[pos] == '{')
	{
		if (is_valid_location(line))
			return (Block::LOCATION);
	}
	return (Block::END_OF_BLOCK);
}

void ConfigFile::check_syntax_server_block(void)
{
	int i;
	bool inServer = false, listenFound = false, serverNameFound = false;
	for (i = 0; i < static_cast<int>(this->_content.size()); i++)
	{
		if (this->_content[i].empty() || is_comment(this->_content[i]))
			continue;
		if (!inServer && this->_content[i] == "server {")
		{
			inServer = true;
			continue;
		}
		Block::ConfigType type = check_syntax_line(this->_content[i], i);
		if (inServer && type == Block::LISTEN_DIRECTIVE)
			listenFound = true;
		else if (inServer && type == Block::SERVER_NAME_DIRECTIVE)
			serverNameFound = true;
		else if (inServer && type == Block::CLIENT_MAX_BODY && convert_client_max_body(this->_content[i]))
			continue;
		else if (inServer && type == Block::DIRECTIVE)
			continue;
		else if (inServer && type == Block::LOCATION)
			i = check_syntax_location_block(this->_content, i);
		else if (inServer && type == Block::END_OF_BLOCK)
			inServer = false;
		else
			throw InvalidSyntaxException(i, INVALID_SYNTAX);
	}
	if (inServer)
		throw InvalidSyntaxException(i - 1, INVALID_SYNTAX);
	if (!listenFound)
		throw InvalidSyntaxException(-1, PORT_NOT_DEFINED);
	if (!serverNameFound)
		throw InvalidSyntaxException(-1, SERVER_NAME_NOT_DEFINED);
}

int ConfigFile::check_syntax_location_block(std::vector<std::string> content, int i)
{
	int index = i + 1;

	for (; index < static_cast<int>(content.size()); index++)
	{
		if (this->_content[index].empty() || is_comment(this->_content[index]))
			continue;
		Block::ConfigType type = check_syntax_line(this->_content[index], i);
		if (type == Block::DIRECTIVE || type == Block::LISTEN_DIRECTIVE || type == Block::SERVER_NAME_DIRECTIVE)
			continue;
		if (type == Block::CLIENT_MAX_BODY && convert_client_max_body(this->_content[index]))
			continue;
		else if (type == Block::END_OF_BLOCK)
			break;
		else
			throw InvalidSyntaxException(index, INVALID_SYNTAX);
	}
	return (index);
}

BodySizeType ConfigFile::get_body_size_type(std::string& str_size)
{
	str_size.pop_back();
	if (str_size.back() == 'm' || str_size.back() == 'M')
	{
		str_size.pop_back();
		return (MEGA);
	}
	else if (str_size.back() == 'k' || str_size.back() == 'K')
	{
		str_size.pop_back();
		return (KILO);
	}
	else if (str_size.back() == 'g' || str_size.back() == 'G')
	{
		str_size.pop_back();
		return (GIGA);
	}
	else if (str_size.back() == 'b' || isdigit(str_size.back()))
	{
		str_size.pop_back();
		return (BYTES);
	}
	else
		return (UNKNOWN);
}

bool ConfigFile::convert_client_max_body(std::string& line)
{
	BodySizeType type = get_body_size_type(line);
	switch (type)
	{
		case MEGA:
			line += "000000;";
			break;
		case GIGA:
			line += "000000000;";
			break;
		case KILO:
			line += "000;";
			break;
		case BYTES:
			line += ";";
			break;
		default:
			return (false);
	}
	return (true);
}

// ========================== EXCEPTION HANDLER ================================

void ConfigFile::handle_syntax_exception(int index, ErrCode err)
{
	switch(err) {
		case INVALID_SYNTAX:
			logger.log(Logger::ERROR) << "invalid syntax at line " << index + 1;
			break;
		case PORT_NOT_DEFINED:
			logger.log(Logger::ERROR) << "`listen` not defined";
			break;
		case SERVER_NAME_NOT_DEFINED:
			logger.log(Logger::ERROR) << "`server_name` not defined";
			break;
		case DUPLICATE_DIRECTIVE:
			logger.log(Logger::ERROR) << "invalid duplicate directive at line " << index + 1;
			break;
		case UNKNOWN_DIRECTIVE:
			logger.log(Logger::ERROR) << "unknown directive at line " << index + 1;
			break;
		case INVALID_VALUES:
			logger.log(Logger::ERROR) << "invalid values at line " << index + 1;
			break;
		case INVALID_PORT_VALUE:
			logger.log(Logger::ERROR) << "invalid port at line " << index + 1;
			break;
		case INVALID_SERVER_NAME:
			logger.log(Logger::ERROR) << "invalid server name at line " << index + 1;
			break;
		default:
			break;
	}
}

// ================================ GETTERS ====================================

std::vector<std::string> ConfigFile::get_content(void) { return (this->_content); }
std::vector<ServerBlock> ConfigFile::get_server_blocks(void) { return (this->_serverBlock); }

// =========================== OPERATOR OVERLOADS ==============================

std::ostream& operator<<(std::ostream& output, ConfigFile& config)
{
	int count = 0;
	for(int i = 0; i < static_cast<int>(config.get_server_blocks().size()); i++)
	{
		output << " ----------\n";
		output << "| SERVER " << count + 1 << " |\n";
		output << " ----------\n";
		output << config.get_server_blocks()[i];
		output << "\n";
		count++;
	}
	return (output);
}
