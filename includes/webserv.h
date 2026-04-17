#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <cerrno>
#include <cstdlib>
#include <stdexcept>
#include "utils.h"

class ConfigException : public std::exception
{
public:
	ConfigException(const std::string &msg) : _msg(msg) {}
	~ConfigException() throw() {}
	const char *what() const throw() { return _msg.c_str(); }
private:
	std::string _msg;
};

typedef struct s_configuration
{
	std::map<int, std::string>	err_pages;
}	t_configuration;

int		parseConf(t_configuration conf, std::string file);
void	parse_err_pages(t_configuration &conf, std::ifstream &file);
