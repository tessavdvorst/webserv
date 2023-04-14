#ifndef SERVER_HPP
# define SERVER_HPP

# include <cstring>
# include <cstdlib>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <netdb.h>
# include <netinet/ip.h>
# include <unistd.h>
# include <fcntl.h>
# include <algorithm>
# include <dirent.h>
# include <iterator>
# include <fstream>
# include <cstddef>
# include <sys/select.h>
# include <sys/time.h>
# include <sys/stat.h>
# include <climits>
# include <cstdio>
# include "ConfigFile.hpp"
# include "utils.hpp"
# include "SocketMonitor.hpp"
# include "Client.hpp"
# include <exception>

#define BACKLOG 128

class ServerBlock;
class Client;

class Server {
	private:
		int _port;
		int _listenFd;
		struct sockaddr_in _serverAddress;
		std::vector<ServerBlock*> _server_config;
		Server();

	public:
		Server(ServerBlock& server);
		~Server();

		class FailedToBindSamePortException: public std::exception {
			public:
				const char* what() const throw() { return ("failed to bind on the same port"); };
		};

		void set_address(void);
		void bind(void);
		void listen(void);
		int accept(void);

		int get_port(void);
		int get_socketfd(void);
		std::vector<ServerBlock*>	get_server_blocks(void);
		void						add_config(ServerBlock *config);
};

#endif
