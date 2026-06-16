#include "webserv.h"

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
		
		file << "POST /upload HTTP/1.1\r\n";
		file << "Host: localhost:8080\r\n";
		file << "User-Agent: curl/7.64.1\r\n";
		file << "Content-Length: 150\r\n";
		file << "Content-Type: text/html\r\n";
		file << "\r\n";
		file << "This is the content of the file being uploaded via POST request.\n";
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
	char    buffer[500];
	write_request_to_file("request.txt");
	int fd = open("request.txt", O_RDONLY);
	Connection conn(fd);
	while ((b_size = read(fd, buffer, 500)) > 0) 
	{
		buffer[b_size] = '\0';
		conn.process(conf.servers.begin()->second, std::string(buffer));
	}
	conn.process(conf.servers.begin()->second, "");
	//update upload counter before exiting
	update_counter(conf.upload_counter_file, conf.servers.begin()->second.upload_counter, 'w');
}