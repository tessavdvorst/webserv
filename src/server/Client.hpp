#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "utils.hpp"
# include "ServerBlock.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "Server.hpp"
# include "CGI.hpp"

enum ClientStatus {
	READY_TO_READ,
	READING_HEADER,
	HEADER_READ,
	HEADER_PARSED,
	FORK_SET_PIPES,
	READY_TO_READ_BODY,
	READING_BODY,
	BODY_TO_PIPE,
	BODY_DONE,
	RUN_CGI,
	RUN_CGI_POST,
	READY_TO_READ_PIPE,
	CGI_HEADER,
	CGI_BODY,
	CGI_TO_PAYLOAD,
	READY_TO_SEND,
	SENDING_BODY,
	ALMOST_DONE,
	FINISHED
};

class Server;
class CGI;

class Client {
	private:
		int						_fd;
		int						_listener;
		ClientStatus			_status;
		std::vector<char>		_header;
		std::vector<char>		_body;
		Server*					_server;
		Request					_request;
		Response				_response;
		CGI*					_cgi;

	public:
		Client();
		Client(int fd, int listener, Server* server);
		Client(const Client& that);
		~Client();

		Client& operator=(const Client& that);

		void				update_status(ClientStatus newStatus);
		void				parse_request(void);
		void				generate_response(void);
		bool				process_buffer(char *buffer, int bytes_read);
		bool				process_cgi_buffer(char *buffer, int bytes_read);
		void				read_header(int fd, bool is_cgi);
		void				read_body(int fd, bool is_cgi);
		int					get_fd(void) const;
		std::string			get_ip(void);
		int					get_listener(void) const;
		ClientStatus		get_status(void) const;
		std::vector<char>	get_header(void) const;
		std::vector<char>	get_body(void) const;
		void				set_body(std::vector<char> body);
		void				print_header();
		void				print_body();
		Response			get_response(void) const;
		Request				get_request(void) const;
		void				set_request_body(std::vector<char> body);
		void				set_response_payload(std::string payload);
		void				set_response_headers(std::string headers);
		CGI					*get_cgi(void) const;
		void				erase_body(std::vector<char>::size_type len);
};

#endif
