#include "webserv.h"

static std::string trim(const std::string &s)
{
	std::size_t start = s.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	std::size_t end = s.find_last_not_of(" \t\r\n");
	return s.substr(start, end - start + 1);
}

void	parse_err_pages(t_configuration &conf, std::ifstream &file)
{
	std::string	line;

	while (std::getline(file, line))
	{
		std::string trimmed = trim(line);
		if (trimmed.empty())
			break;

		std::size_t colon_pos = trimmed.find(':');
		if (colon_pos == std::string::npos)
			throw ConfigException("Invalid Configuration Token at err_pages: " + trimmed);

		std::string code_str = trim(trimmed.substr(0, colon_pos));
		std::string path     = trim(trimmed.substr(colon_pos + 1));

		if (code_str.empty() || path.empty())
			throw ConfigException("Invalid Configuration Token at err_pages: " + trimmed);

		errno = 0;
		char *endptr;
		int key = static_cast<int>(std::strtol(code_str.c_str(), &endptr, 10));
		if (errno == ERANGE)
			throw ConfigException("Error code out of range at err_pages: " + code_str);
		if (endptr == code_str.c_str() || *endptr != '\0')
			throw ConfigException("Non-numeric error code at err_pages: " + code_str);
		if (key < 100 || key > 599)
			throw ConfigException("HTTP error code must be 100-599 at err_pages: " + code_str);

		conf.err_pages[key] = path;
	}
}

int	parseConf(t_configuration conf, std::string file)
{
	(void)conf;
	(void)file;
	return 0;
}