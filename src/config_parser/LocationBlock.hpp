#ifndef LOCATIONBLOCK_HPP
#define LOCATIONBLOCK_HPP

#include <iostream>
#include <string>
#include <map>
#include "Block.hpp"
#include "utils.hpp"

class LocationBlock: public Block {
	private:
		std::string _path;

	public:
		LocationBlock();
		LocationBlock(const LocationBlock& that);
		LocationBlock(const std::string& path);
		~LocationBlock();

		LocationBlock& operator=(const LocationBlock& that);
		
		void save_location(std::vector<std::string> content, int i);
		// std::vector<std::string> get_directive(std::string directive);
		std::string get_path(void) const;
};

#endif
