#include "ServerBlock.hpp"
#include "ConfigFile.hpp"

// ============================= CONSTRUCTOR ===================================

ServerBlock::ServerBlock(): Block(), _port(-1) {}

ServerBlock::ServerBlock(const ServerBlock& that)
{
	*this = that;
}

// ============================= DESTRUCTOR ===================================

ServerBlock::~ServerBlock() {}

// ====================== ASSIGN OPERATOR OVERLOAD ============================

ServerBlock& ServerBlock::operator=(const ServerBlock& that)
{
	Block::operator =(that);
	this->_port = that._port;
	this->_server_name.erase(this->_server_name.begin(), this->_server_name.end());
	this->_server_name.insert(this->_server_name.end(), that._server_name.begin(), that._server_name.end());
	this->_locationBlock.erase(this->_locationBlock.begin(), this->_locationBlock.end());
	this->_locationBlock.insert(this->_locationBlock.end(), that._locationBlock.begin(), that._locationBlock.end());
	return (*this);
}

// ============================ SAVE LOCATION =================================

void ServerBlock::save_location(std::vector<std::string> content, int i)
{
	std::string arg;
	std::stringstream ss(content[0]);

	std::vector<std::string> args;
	while (getline(ss, arg, ' '))
		args.push_back(arg);

	LocationBlock location(args[1]);
	location.save_location(content, i);
	this->_locationBlock.push_back(location);
}

// ============================= SAVE PORT ====================================

int ServerBlock::convert_port(const char* input)
{
	char* pEnd;
	long int val = strtol(input, &pEnd, 10);
	if (*pEnd != '\0' || val < INT_MIN || val > 65536)
		return (-1);
	return (static_cast<int>(val));
}

void ServerBlock::save_port(const std::string& line, int index)
{
	if (line.find_first_of(" ") == std::string::npos)
		throw ConfigFile::InvalidSyntaxException(index, ConfigFile::INVALID_VALUES);

	int value;
	std::string arg;
	std::string directive;
	std::istringstream iss(line);

	std::getline(iss, directive, ' ');
	std::getline(iss, arg, ' ');
	size_t pos = arg.find(';');
	if (pos != std::string::npos)
		arg.erase(pos, 1);
	if (is_comment(arg) || (value = convert_port(arg.c_str())) == -1)
		throw ConfigFile::InvalidSyntaxException(index, ConfigFile::INVALID_PORT_VALUE);
	if (this->_port != -1)
		throw ConfigFile::InvalidSyntaxException(index, ConfigFile::DUPLICATE_DIRECTIVE);
	this->_port = value;
}

// ========================= SAVE SERVER_NAME =================================

void ServerBlock::save_server_name(const std::string& line, int index)
{
	
	if (line.find_first_of(" ") == std::string::npos)
		throw ConfigFile::InvalidSyntaxException(index, ConfigFile::INVALID_VALUES);

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
	if (args.empty())
		throw ConfigFile::InvalidSyntaxException(index, ConfigFile::INVALID_SERVER_NAME);
	if (!this->_server_name.empty())
		throw ConfigFile::InvalidSyntaxException(index, ConfigFile::DUPLICATE_DIRECTIVE);
	this->_server_name = args;
}

// =========================== FIND DIRECTIVE =================================

LocationBlock ServerBlock::get_location(std::string url)
{
	for (std::vector<LocationBlock>::iterator block = this->_locationBlock.begin(); block != this->_locationBlock.end(); block++) {
		if (url.find(block->get_path()) == 0)
			return (*block);
	}
	throw ServerBlock::NoLocationException();
}

std::vector<std::string> ServerBlock::get_directive(std::string directive, std::string url)
{
	std::map<std::string, std::vector<std::string> > directives;

	try {
		directives = this->get_location(url).get_directives();
		if (directives.count(directive) > 0)
			return (directives[directive]);
		else
			throw ServerBlock::NoLocationException();
	} catch (ServerBlock::NoLocationException &e) {
		directives = this->get_directives();
	}

	if (directives.count(directive) > 0)
		return (directives[directive]);

	std::vector<std::string> empty;
	empty.push_back("");
	return (empty);
}

// =============================== GETTER =====================================

int ServerBlock::get_port(void) const
{
	return (this->_port);
}

std::vector<std::string> ServerBlock::get_server_name(void)
{
	return (this->_server_name);
}

// =========================== PRINT FUNCTIONS ================================

void ServerBlock::print_location(void) const
{
	int count = 0;

	for(int i = 0; i < static_cast<int>(this->_locationBlock.size()); i++)
	{
		std::cout << "LOCATION " << this->_locationBlock[i].get_path() << "\n";
		std::cout << "---------------\n";
		this->_locationBlock[i].print_directives();
		std::cout << "\n";
		count++;
	}
}

// ====================== INSERTION OPERATOR OVERLOAD ============================

std::ostream& operator<<(std::ostream& output, ServerBlock& server)
{
	output << std::left << std::setw(25)<< "listen";
	output << server.get_port() << '\n';
	output << std::left << std::setw(25) << "server_names";
	for (int i = 0; i < static_cast<int>(server.get_server_name().size()); i++)
		output << server.get_server_name()[i] << ' ';
	output << "\n\n";
	server.print_directives();
	output << "\n";
	server.print_location();
	return (output);
}

std::vector<LocationBlock> ServerBlock::get_locations(void) const {
	return (this->_locationBlock);
}
