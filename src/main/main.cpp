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

	CoreServer core;
	bool bound_any = false;

	// A single "server" block may itself listen on more than one host:port
	// pair (its `ports` vector), and the config as a whole may define more
	// than one server block, so every port of every server needs its own
	// listening socket added to the shared epoll instance.
	for (std::map<std::string, t_server>::iterator sit = conf.servers.begin();
		 sit != conf.servers.end(); ++sit)
	{
		t_server &srv_cfg = sit->second;

		if (srv_cfg.ports.empty())
		{
			p_error("Server block has no listen directive: " + srv_cfg.server_name);
			continue;
		}

		for (size_t i = 0; i < srv_cfg.ports.size(); ++i)
		{
			int         port = srv_cfg.ports[i].first;
			std::string host = srv_cfg.ports[i].second.empty() ? "0.0.0.0" : srv_cfg.ports[i].second;

			if (core.addServer(host, port, srv_cfg))
				bound_any = true;
			else
				p_error("CoreServer: failed to bind " + host + ":" + num_to_str(port));
		}
	}

	if (!bound_any)
	{
		p_error("No listening sockets could be created, exiting.");
		return (1);
	}

	core.run();

	//update upload counter before exiting
	update_counter(conf.upload_counter_file, conf.servers.begin()->second.upload_counter, 'w');
}
