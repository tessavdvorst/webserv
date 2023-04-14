#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "utils.hpp"
# include "Request.hpp"
# include <fstream>
# include <fcntl.h>
# include <dirent.h>

enum RequestType
{
	GET,
	POST,
	DELETE,
	NOT_ALLOWED
};

class Response {
	private:
		std::string _index_url;
		std::fstream _infile;
		std::string _headers;
		std::string _payload;
		bool		_need_cgi;
		RequestType	_request_type;

		Request _request;

	public:
		Response();
		Response(Request request);
		Response(const Response& that);
		~Response();

		Response& operator=(const Response& that);

		class CGIDeferer : public std::exception {
			public:
				const char *what() const _NOEXCEPT { return "Defer to CGI\n"; }
		};

		class HTTPException : public std::exception {
			private:
				std::string	_http_error;
				std::string	_redir_target;
				bool		_is_soft_error;

			public:
				HTTPException(std::string http_error) : _http_error(http_error), _redir_target(""), _is_soft_error(true) {}
				HTTPException(std::string http_error, bool is_soft_error) : _http_error(http_error), _redir_target(""), _is_soft_error(is_soft_error) {}
				HTTPException(std::string http_error, std::string redir_target) : _http_error(http_error), _redir_target(redir_target), _is_soft_error(true) {}
				HTTPException(std::string http_error, std::string redir_target, bool is_soft_error) : _http_error(http_error), _redir_target(redir_target), _is_soft_error(is_soft_error) {}
				~HTTPException() _NOEXCEPT {}
				
				bool 		is_soft(void) const { return (this->_is_soft_error); }
				std::string	get_error_code(void) const { return (this->_http_error); }
				std::string	get_target(void) const { return (this->_redir_target); }
		};
		
		RequestType check_request_type(void);
		void		handle_request(void);
		void		handle_error_pages(Response::HTTPException &e);
		void		serve_config_error_page(Response::HTTPException &e);
		std::string get_error_msg(std::string error_code);
		void		serve_directory_listing(void);

		bool		need_cgi(void);

		std::string get_file_data(void);
		std::string	get_payload(void);
		std::string	get_headers(void);
		RequestType get_request_type(void);

		void		set_payload(std::string payload);
		void		set_headers(std::string headers);
};

#endif
