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

void write_request_to_file(const std::string &filename)
{
    std::ofstream file(filename.c_str());
    if (!file.is_open())
        throw std::runtime_error("Cannot open file: " + filename);
		
		file << "GET /images HTTP/1.1\r\n";
		file << "Host: localhost:8080\r\n";
		file << "User-Agent: curl/7.64.1\r\n";
		file << "Content-Length: 150\r\n";
		file << "Content-Type: text/html\r\n";
		file << "\r\n";
    file.close();
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
	ssize_t b_size = 0;
	char    buffer[BUFFER_SIZE + 1];
	write_request_to_file("request.txt");
	int fd = open("request.txt", O_RDONLY);
	Handler handler(fd);
	while ((b_size = read(fd, buffer, BUFFER_SIZE + 1)) > 0) 
	{
		buffer[b_size] = '\0';
		handler.process(conf.servers.begin()->second, std::string(buffer)); // this is ass coz you only have to call parse.request
		if (handler.state == COMPLETE || handler.state == ERROR)
			break;
	}
	// this looks stupid but it is to make sure the request is processed if it was not complete yet ( in case of body smaller than content-length )
		// handler.process(conf.servers.begin()->second, "");
	//update upload counter before exiting
	update_counter(conf.upload_counter_file, conf.servers.begin()->second.upload_counter, 'w');

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
}