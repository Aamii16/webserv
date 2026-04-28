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
		
		file << "POST /index.html HTTP/1.1\r\n";
		file << "Host: localhost:8080\r\n";
		file << "User-Agent: curl/7.64.1\r\n";
		file << "Content-Length: 124\r\n";
		file << "\r\n";
    
    file.close();
}

void print_request(const Request &req)
{
    std::cout << "=== HTTP REQUEST ===" << std::endl;
	std::string method_str;
	if (req.getMehod() == GET)
		method_str = "GET";
	else if (req.getMehod() == POST)
		method_str = "POST";
	else if (req.getMehod() == DELETE)
		method_str = "DELETE";
	// etc
	std::cout << "Method:" << method_str << std::endl;
    std::cout << "Target:" << req.getTarget() << std::endl;
    std::cout << "Version:" << req.getVersion() << std::endl;
    
    std::cout << "--- Headers ---" << std::endl;
	strstrMap tmp = req.getHeaders();
	for (strstrMap::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
        std::cout << it->first << ":" << it->second << std::endl;
    
    std::cout << "\n--- Body ---" << std::endl;
    std::cout << req.getBody() << std::endl;
	std::cout << "state: " << req.getState() << std::endl;
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
	// std::string request;
	Request	request;
	write_request_to_file("request.txt");
	int fd = open("request.txt", O_RDONLY);
	while ((b_size = read(fd, buffer, 500)) > 0) 
	{
		buffer[b_size]='\0';
		request.parse_request(std::string(buffer));
		if (request.getState() == BAD_REQUEST)
		{
			std::cout << "bad reuqest\n";
			return (1); // send REsponse with Error_code
		}
		if (request.getState() == COMPLETE || request.getState() == BAD_REQUEST)
			request.process(conf);
	}
	print_request(request);
}