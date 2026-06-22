#include "webserv.h"
#include "core_server.hpp"

bool	validConf(std::string file_path, std::string ext, std::ifstream &file)
{
	int	valid;

	valid = file_path.compare(file_path.length() - ext.length(), ext.length(), ext);
	if (valid)
		return (p_error("Invalid configuration file extension!"), false);
	valid = 1;
	file.open(file_path.c_str());
	if (!file.is_open())
		return (p_error("Unable to open configuration file!"), false);
	return (true);
}

int	main(int ac, char **av)
{
	t_configuration conf;
	std::ifstream	file;

	if (ac != 2)
		return (p_error("Configuration file required!"), 1);
	if (!validConf(std::string(av[1]), std::string(".conf"), file))
		return (1);
	parseConf(conf, file);

	t_server &srv_cfg = conf.servers.begin()->second;
	CoreServer core;
	if (!core.addServer(srv_cfg.ip.empty() ? "0.0.0.0" : srv_cfg.ip,
	                    srv_cfg.port,
	                    srv_cfg))
	{
		p_error("CoreServer: failed to bind");
		return (1);
	}

	core.run();

	//update upload counter before exiting
	update_counter(conf.upload_counter_file, conf.servers.begin()->second.upload_counter, 'w');
}