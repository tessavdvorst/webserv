#ifndef BLOCK_HPP
#define BLOCK_HPP

# include <iomanip>
#include <map>
#include <vector>
#include <sstream>
#include <exception>
#include "utils.hpp"

class Block {
	protected:
		std::map<std::string, std::vector<std::string> > _directives;

	public:
		Block();
		Block(const Block& that);
		virtual ~Block();
		
		Block& operator=(const Block& that);

		enum ConfigType {
			LOCATION,
			LISTEN_DIRECTIVE,
			SERVER_NAME_DIRECTIVE,
			CLIENT_MAX_BODY,
			DIRECTIVE,
			END_OF_BLOCK,
			UNKNOWN
		};

		int get_first_direc_pos(const std::vector<std::string>& config, int currentPos);
		int get_type_index(const std::string& str);
		ConfigType get_direc_type(const std::string& direc);
		bool replace_dup_direc(std::pair<std::string, std::vector<std::string> > newPair);
		void save_directive(const std::string& line, int i);
		
		virtual void save_location(std::vector<std::string> content, int i) = 0;
		
		// virtual std::vector<std::string> get_directive(std::string directive) = 0;
		void print_directives(void) const;

		std::map<std::string, std::vector<std::string> > get_directives(void);

};

#endif
