#include "Block.hpp"
#include "ConfigFile.hpp"

// ============================= CONSTRUCTOR ===================================

Block::Block() {}

Block::Block(const Block& that)
{
	*this = that;
}

// ============================= DESTRUCTOR ===================================

Block::~Block() {}

// ====================== ASSIGN OPERATOR OVERLOAD ============================

Block& Block::operator=(const Block& that) {
	this->_directives.erase(this->_directives.begin(), this->_directives.end());
	this->_directives.insert(that._directives.begin(), that._directives.end());
	return (*this);
}

// =========================== SAVE DIRECTIVE =================================

int Block::get_first_direc_pos(const std::vector<std::string>& config, int currentPos)
{
	while (currentPos < static_cast<int>(config.size()))
	{
		if (config[currentPos] == "server {" 
			|| config[currentPos].substr(0, strlen("location")) == "location"
			|| config[currentPos].empty() || is_comment(config[currentPos]))
			currentPos++;
		else
			break;
	}
	return (currentPos);
}

int Block::get_type_index(const std::string& str)
{
	std::string types[] = {"listen", "server_name", "root", "deny",\
							"index", "error_page404", "error_page405", "error_page500", "client_max_body_size", \
							"autoindex", "return", "allowed_methods", "redirect", "cgi_pass"};
	int size = sizeof(types)/sizeof(types[0]);

	for (int index = 0; index < size; index++)
	{
		if (str.substr(0, strlen(types[index].c_str()) + 1) == types[index] + ' ')
			return (index);
	}
	return (-1);
}

Block::ConfigType Block::get_direc_type(const std::string& direc)
{
	int index = get_type_index(direc);
	if (index == -1)
	{
		if (direc.substr(0, strlen("location")) == "location")
			return (LOCATION);
		else if (direc == "}") // know for sure that the syntax is correct because prior syntax check
			return (END_OF_BLOCK);
		else
			return (UNKNOWN);
	}
	if (direc.substr(0, strlen("listen")) == "listen")
		return (LISTEN_DIRECTIVE);
	else if (direc.substr(0, strlen("server_name")) == "server_name")
		return (SERVER_NAME_DIRECTIVE);
	return (DIRECTIVE);
}

bool Block::replace_dup_direc(std::pair<std::string, std::vector<std::string> > pair)
{
	if (pair.first == "listen" || pair.first == "server_name")
		return (false);
	this->_directives.erase(this->_directives.find(pair.first));
	this->_directives.insert(pair);
	return (true);
}

void Block::save_directive(const std::string& line, int i)
{
	if (line.find_first_of(" ") == std::string::npos)
		throw ConfigFile::InvalidSyntaxException(i, ConfigFile::INVALID_VALUES);

	std::string arg;
	std::string directive;
	std::istringstream iss(line);
	std::vector<std::string> args;

	std::getline(iss, directive, ' ');
	while (std::getline(iss, arg, ' '))
	{
		size_t pos = arg.find(';');
		if (pos != std::string::npos)
			arg.erase(pos, 1);
		if (is_comment(arg))
			break;
		args.push_back(arg);
	}

	std::pair<std::map<std::string, std::vector<std::string> >::iterator,bool> ret;
	ret = this->_directives.insert(std::make_pair(directive, args));
	if (ret.second == false)
	{
		if (directive == "root" || !replace_dup_direc(std::make_pair(directive, args)))
			throw ConfigFile::InvalidSyntaxException(i, ConfigFile::DUPLICATE_DIRECTIVE);
		else
			Logger::log(Logger::WARN) << "duplicate directive at line " << i + 1;
	}
}

// =============================== GETTER =====================================

std::map<std::string, std::vector<std::string> > Block::get_directives(void)
{
	return (this->_directives);
}

// =========================== PRINT FUNCTIONS ================================

void Block::print_directives(void) const
{
	std::map<std::string, std::vector<std::string> >::const_iterator mapIt = this->_directives.begin();
	for (; mapIt != this->_directives.end(); mapIt++)
	{
		std::cout << std::left << std::setw(25) << mapIt->first;
		for (std::vector<std::string>::const_iterator vecIt = mapIt->second.begin(); vecIt != mapIt->second.end(); vecIt++)
			std::cout << *vecIt << ' ';
		std::cout << "\n";
	}
}
