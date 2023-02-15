#include "gopher++.h"
#include <iostream>

int main(int argc, char **argv) {
	std::string config_file;
	if (argc == 1)
		config_file = "gopher++.conf";
	else if (argc > 2)
		std::cout << "Too many arguments. Trying to use the first one as config file..." << std::endl;
	if (argc > 1)
		config_file = argv[1];
	// ...
	return (0);
}
