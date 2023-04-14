#ifndef CGI_HPP
# define CGI_HPP

#include <fcntl.h>
#include "utils.hpp"
#include "Request.hpp"
#include "Response.hpp"

class Request;
class Response;

class CGI {
	private:
		pid_t				_pid;
		int					_pipe_out[2];
		int					_pipe_in[2];
		std::vector<char *>	_args;
		std::vector<char *>	_env;
		std::string			_content_length;
		std::vector<char>	_header;
		std::vector<char>	_body;

		Request _request;
		Response _response;

	public:
		CGI(); // Here because we can't construct a Client if we don't have it.
		CGI(Request request, Response response);
		CGI(const CGI& that);
		~CGI();

		CGI& operator=(const CGI& that);
		void run(void);

		void init_env_args(void);
		void set_args(std::string directive);
		void set_env(std::string directive);

		std::vector<char *>	get_args(void);
		int					get_input_read_fd();
		int					get_input_write_fd();
		int					get_output_read_fd();
		int					get_output_write_fd();
		std::vector<char>	get_header(void);
		std::vector<char>	get_body(void);
		pid_t				get_pid(void);
		void				set_header(std::vector<char> header);
		void				set_body(std::vector<char> body);
		void				set_request(Request request);

		void 				erase_body();
		void 				erase_header();
};

#endif
