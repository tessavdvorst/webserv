#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "utils.hpp"
# include "ServerBlock.hpp"
# include <sys/stat.h>
# include <map>
# include <algorithm>
# include <iterator>

class Request {
	private:
		std::vector<char>			_packet_header;
		std::vector<char>			_packet_body;
		std::string					_method;
		std::string					_url;
		std::string					_decoded_url;
		std::string					_index_url;
		std::string					_local_url;
		std::string					_full_url;
		std::string					_protocol;
		std::string					_host;
		std::string					_content_type;
		std::string					_port;
		std::vector<ServerBlock*>	_config;

	public:
		Request();
		Request(std::vector<char> header, std::vector<char> body, std::vector<ServerBlock*> config);
		Request(const Request& that);
		~Request();

		Request& operator=(const Request& that);

		class NoLocationException : public std::exception {
			public:
				const char *what() const _NOEXCEPT { return ("No such location!"); }
		};

		std::string					url_decode(std::string in);
		void						parse_request(void);
		void						parse_request_line(std::stringstream& in);
		void						parse_header(std::stringstream& in);
		ServerBlock					*get_server(void);
		std::vector<std::string>	get_directive(std::string directive);
		void						parse_url(void);
		bool						is_dir(void);
		std::string					get_url(void);
		std::string					get_index_url(void);
		std::string					get_decoded_url(void);
		std::string					get_full_url(void);
		std::string					get_local_url(void);
		std::string					get_request_url(void);
		std::string					get_method(void);
		std::string					get_protocol(void);
		std::string					get_host(void);
		std::string					get_content_type(void);
		std::string					get_port(void);
		std::vector<char>			get_packet_body(void);
		void						set_packet_body(std::vector<char> body);
};

#endif
