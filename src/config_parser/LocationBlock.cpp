#include "LocationBlock.hpp"
#include "ConfigFile.hpp"

// ============================= CONSTRUCTOR ===================================

LocationBlock::LocationBlock(): Block() {}

LocationBlock::LocationBlock(const LocationBlock& that)
{
	*this = that;
}

LocationBlock::LocationBlock(const std::string& path): Block(), _path(path) {}

// ============================= DESTRUCTOR ===================================

LocationBlock::~LocationBlock() {}

// ====================== ASSIGN OPERATOR OVERLOAD ============================

LocationBlock& LocationBlock::operator=(const LocationBlock& that)
{
	Block::operator =(that);
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

std::string LocationBlock::get_path(void) const { return (this->_path); }
