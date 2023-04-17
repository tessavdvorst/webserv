#ifndef SERVERBLOCK_HPP
#define SERVERBLOCK_HPP

#include "LocationBlock.hpp"
#include "utils.hpp"

#ifdef __linux__
#define _NOEXCEPT noexcept
#endif

class ServerBlock: public Block {
	private:
		int							_port;
		std::string					_ip;
		std::vector<std::string>	_server_name;
		std::vector<LocationBlock>	_locationBlock;

	public:
		ServerBlock();
		ServerBlock(const ServerBlock& that);
		~ServerBlock();

		ServerBlock& operator=(const ServerBlock& that);

		class NoLocationException : public std::exception {
			public:
				const char *what() const _NOEXCEPT { return ("No such location!"); }
		};

		void						save_location(std::vector<std::string> content, int i);
		void						print_location(void) const;

		int							convert_port(const char* input);
		void						save_listen(const std::string& line, int index);
		void						save_server_name(const std::string& line, int index);

		int							get_port(void) const;
		std::string					get_ip(void) const;
		std::vector<std::string>	get_server_name(void);
		std::vector<LocationBlock>	get_locations(void) const;

		std::vector<std::string>	get_directive(std::string directive, std::string url);
		LocationBlock				get_location(std::string url);
};

std::ostream& operator<<(std::ostream& output, ServerBlock& server);

#endif
