#include "LocationBlock.hpp"
#include "ConfigFile.hpp"

// ============================= CONSTRUCTOR ===================================

LocationBlock::LocationBlock(): Block() {}

LocationBlock::LocationBlock(const LocationBlock& that): Block(that)
{
	*this = that;
}

LocationBlock::LocationBlock(const std::string& path): Block(), _path(path) {}

// ============================= DESTRUCTOR ===================================

LocationBlock::~LocationBlock() {}

// ====================== ASSIGN OPERATOR OVERLOAD ============================

LocationBlock& LocationBlock::operator=(const LocationBlock& that)
{
	this->_path = that._path;
	return (*this);
}

// ============================ SAVE LOCATION =================================

void LocationBlock::save_location(std::vector<std::string> content, int i)
{
	for (int index = 0; index < static_cast<int>(content.size()); index++)
	{
		index = get_first_direc_pos(content, index);
		if (index == static_cast<int>(content.size()))
			return ;
		while (index < static_cast<int>(content.size()))
		{
			while (index < static_cast<int>(content.size())
				&& (content[index].empty() || is_comment(content[index])))
				index++;
			ConfigType dType = get_direc_type(content[index]);
			if (dType == Block::DIRECTIVE)
				save_directive(content[index], i + index);
			else if (dType == Block::END_OF_BLOCK)
				return;
			else
				throw ConfigFile::InvalidSyntaxException(i + index, ConfigFile::UNKNOWN_DIRECTIVE);
			index++;
		}
	}
}

// =============================== GETTER =====================================


// std::vector<std::string> LocationBlock::get_directive(std::string directive)
// {
// 	std::map<std::string, std::vector<std::string> > directives;
	
// 	try {
// 		directives = this->get_location("/").get_directives();
// 	} catch (ServerBlock::NoLocationException e) {
// 		directives = this->get_directives();
// 	}

// 	if (directives.empty())
// 		std::cout << "EMPTY\n";

// 	if (directives.count(directive) > 0) { // directive present in list
// 		std::cout << "Found " << directive << ": " << directives[directive][0] << "!" << std::endl;
// 		return (directives[directive]);
// 	} else
// 		std::cout << "Missed directive!" << std::endl;
	
// 	std::vector<std::string> empty;
// 	return (empty);
// }

std::string LocationBlock::get_path(void) const { return (this->_path); }
