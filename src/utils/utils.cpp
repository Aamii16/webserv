#include "webserv.h"


std::string	num_to_str(long long num)
{
	std::stringstream ss;
	ss << num;
	std::string num_str;
	ss >> num_str;
	return num_str;
}

void	p_error(std::string err)
{
	std::cerr << err << std::endl;
}