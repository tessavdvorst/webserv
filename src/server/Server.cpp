#include "Server.hpp"
#include "ServerBlock.hpp"
#include "utils.hpp"

// ============================= CONSTRUCTOR ===================================

Server::Server() {}

Server::Server(ServerBlock& server): _port(server.get_port()), _listenFd(-1)
{
	int optValReuseAddress = 1;
	ServerBlock* config = new ServerBlock(server);
	this->_server_config.push_back(config);

	set_address();
	if ((this->_listenFd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		error_and_close("failed to create socket for server!", -1);
	fcntl(this->_listenFd, F_SETFL, O_NONBLOCK);

	if (setsockopt(this->_listenFd, SOL_SOCKET, SO_REUSEADDR, &optValReuseAddress, sizeof(optValReuseAddress)) < 0)
		error_and_close("failed to set SO_REUSEADDR option!", this->_listenFd);
}

// ================================ DESTRUCTOR =====================================

Server::~Server()
{
	for (std::vector<ServerBlock*>::iterator it = this->_server_config.begin(); it != this->_server_config.end(); ++it)
		delete *it;
}

void Server::set_address(void)
{
	memset(&this->_serverAddress, 0, sizeof(this->_serverAddress));
	inet_pton(AF_INET, this->_server_config[0]->get_ip().c_str(), &(this->_serverAddress.sin_addr));
	this->_serverAddress.sin_family = AF_INET;
	this->_serverAddress.sin_port = htons(this->_server_config[0]->get_port());
//	this->_serverAddress.sin_addr.s_addr = INADDR_ANY;
}

// ============================ SOCKET METHODS =================================

void Server::bind(void)
{
	if (::bind(this->_listenFd, (struct sockaddr *)&this->_serverAddress, sizeof(this->_serverAddress)) < 0)
	{
		if (errno == EADDRINUSE)
		{
			close(this->_listenFd);
			throw FailedToBindSamePortException();
		}
		error_and_close("failed to bind socket!", this->_listenFd);
	}
}

void Server::listen(void)
{
	if (::listen(this->_listenFd, BACKLOG) < 0)
		return (error_and_close("failed to listen on socket!", this->_listenFd));

	Logger::log(Logger::INFO) << "Listening on port " << this->_server_config[0]->get_port() << "...\n";
}

int Server::accept(void)
{
	int clientFd;
	struct sockaddr_storage remoteAddr;
	socklen_t addrSize = sizeof(remoteAddr);

	if ((clientFd = ::accept(this->_listenFd, (struct sockaddr *)&remoteAddr, &addrSize)) < 0)
		throw std::runtime_error("Failed to connect to client!");
	return (clientFd);
}

// ================================== GETTERS ===========================================

int Server::get_port(void) { return (this->_port); }

std::string Server::get_ip(void) { return (this->_server_config[0]->get_ip()); }

int Server::get_socketfd(void) { return (this->_listenFd); }

std::vector<ServerBlock*> Server::get_server_blocks(void) { return (this->_server_config); }

// ================================== GETTERS ===========================================

void	Server::add_config(ServerBlock *config) {
	this->_server_config.push_back(config);
}
