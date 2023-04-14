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
	std::string config_file;
	if (argc > 2) {
		logger.log(Logger::ERROR) << "invalid amount of arguments";
		return (1);
	} else if (argc == 1) {
		config_file = "/Users/rrinia/dev/gopher++/config_files/gopher++.config";
	} else {
		config_file = std::string(argv[1]);
    }
	if (!check_extension(config_file, ".config"))
		logger.log(Logger::ERROR) << "invalid file extension";
	else
	{
		try {
			ConfigFile configFile(config_file);
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
