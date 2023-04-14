#include "ConfigFile.hpp"
#include "utils.hpp"

Logger logger;

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
			std::vector<std::string> directive = configFile.get_server_blocks()[0].get_directive("root", "/");
		} catch (const ConfigFile::InvalidSyntaxException &e) {
			ConfigFile::handle_syntax_exception(e.getIndex(), e.getErrorCode());
		} catch (const std::exception &e) { //merge these exceptions?
			logger.log(Logger::ERROR) << e.what();
		}
	}
	return (0);
}
