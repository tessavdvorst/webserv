#include "Server.hpp"

Logger logger;

void start_server(ConfigFile* config)
{
	try {
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
				delete server;
				continue ;
			}
			monitor.add_server(server);
			server = NULL;
		}
		monitor.run();
	} catch (const std::exception &e) {
		logger.log(Logger::ERROR) << e.what();
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
	}
	return (0);
}
