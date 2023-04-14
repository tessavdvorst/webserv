#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>
#include <exception>
#include <dirent.h>
#include <iterator>
#include <fstream>
#include <cstddef>
#include <vector>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <climits>
#include <cstdio>
#include "ConfigFile.hpp"
#include "utils.hpp"
#include "Server.hpp"

Logger logger;

void start_server(ConfigFile* config)
{
	try {
	// initialize servers
		SocketMonitor monitor;
		for (int i = 0; i < static_cast<int>(config->get_server_blocks().size()); i++)
		{
			Server *server = new Server(config->get_server_blocks()[i]);
			try {
				server->bind();
				server->listen();
			} catch (const Server::FailedToBindSamePortException &e) {
				monitor.add_server_config(config->get_server_blocks()[i]);
				delete server;
				continue;
			} catch (const std::exception &e) {
				logger.log(Logger::ERROR) << "exception caught";
				delete server;
				continue ;
			}
			monitor.add_server(server);
			server = NULL;
		}
		monitor.run();
	} catch (const std::exception &e) {
		logger.log(Logger::ERROR) << e.what(); //do something
	}
}

int main(int argc, char *argv[])
{
	if (argc != 2)
		logger.log(Logger::ERROR) << "invalid amount of arguments"; 
	else if (!check_extension(argv[1], ".config"))
		logger.log(Logger::ERROR) << "invalid file extension";
	else
	{
		try {
			ConfigFile configFile(argv[1]);
			std::cout << configFile; // print config file
			start_server(&configFile);
		} catch (const ConfigFile::InvalidSyntaxException &e) {
			ConfigFile::handle_syntax_exception(e.getIndex(), e.getErrorCode());
		} catch (const std::exception &e) {
			logger.log(Logger::ERROR) << e.what();
		}
		//close and free servers
	}
	return (0);
}
