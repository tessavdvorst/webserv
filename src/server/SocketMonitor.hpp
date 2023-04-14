#ifndef SOCKETMONITOR_HPP
# define SOCKETMONITOR_HPP

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include "Request.hpp"
#include "Response.hpp"
#include "Client.hpp"

class Server;
class Client;
class CGI;

class SocketMonitor {
	private:
		int _fdMax;
		fd_set _master;
		fd_set _read_fds;
		fd_set _write_fds;
		std::map<int, Server*> _servers;
		std::map<int, Client*> _clients;
		struct timeval _zero_time; //why not set to NULL?

	public:
		SocketMonitor();
		SocketMonitor(const SocketMonitor& that);
		~SocketMonitor();

		SocketMonitor& operator=(const SocketMonitor& that);

		class CloseFdException : public std::exception {
			private:
				int _fd;
			public:
				CloseFdException(int fd) : _fd(fd) {}

				int get_fd(void) const { return (this->_fd); }
		};

		void add_server(Server *server);
		void add_server_config(ServerBlock& server);
		void run(void);
		void accept_connection(int fd);
		void response_handler(Client* client);
		void handle_body(Client* client);
		void fix_payload(Client *client);
		void final_packet(Client *client);

		void add_fd(int fd);
		void remove_fd(int fd);
		void close_and_remove(int fd);
};

#endif

