#include "CGIHandler.hpp"

CGIHandler::~CGIHandler()
{
	for (size_t i = 0; i < vars.size(); ++i)
		delete[] env[i];
	delete[] env;
}

void	convert(std::string &str)
{
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (str[i] == '-')
			str[i] = '_';
		else
			str[i] = std::toupper(str[i]);
	}
}

void	CGIHandler::setEnvVars(const Request& request, const std::string& script_path, const std::string &script_name , const t_server& server)
{
	vars.clear();
	vars.push_back("REQUEST_METHOD=" + request.getMethodString());
	vars.push_back("GATEWAY_INTERFACE=CGIHandler/1.1");
	vars.push_back("SERVER_NAME="+ server.server_name);
	vars.push_back("SERVER_PORT=" + num_to_str(server.port));
	vars.push_back("REQUEST_URI=" + request.getTarget() + request.getQuery());
	vars.push_back("SERVER_PROTOCOL=" + request.getVersion());
	vars.push_back("QUERY_STRING=" + request.getQuery());
	vars.push_back("SCRIPT_NAME=" + script_name);
	vars.push_back("SCRIPT_FILENAME=" + script_path);

	strstrMap headers = request.getHeaders();
	for (strstrMap::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		std::string header_name = it->first;
		convert(header_name);
		vars.push_back("HTTP_" + header_name + "=" + it->second);
	}
	env = new char*[vars.size() + 1];
	for (size_t i = 0; i < vars.size(); ++i)
	{
		env[i] = new char[vars[i].size() + 1];
		std::strcpy(env[i], vars[i].c_str());
	}
	env[vars.size()] = NULL;
}

void	CGIHandler::execute()
{
	
}
