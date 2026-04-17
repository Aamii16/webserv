#include "webserv.h"

int	validConf(std::string file_path, std::string ext)
{
	int	valid;
	std::ifstream	file;

	valid = file_path.compare(file_path.length() - ext.length(), ext.length(), ext);
	if (valid)
		return (p_error("Invalid configuration file extension!"), 0);
	valid = 1;
	file.open(file_path.c_str());
	if (!file.is_open())
		return (p_error("Unable to open configuration file!"), 0);
	file.close();
	return (1);
}

int	main(int ac, char **av)
{
	t_configuration conf;

	if (ac != 2)
		return (p_error("Configuration file required!"), 1);
	if (!validConf(std::string(av[1]), std::string(".conf")))
		return (1);
	parseConf(conf, std::string(av[1]));
}