#pragma once
#include "Request.hpp"
#include <sys/wait.h>
#include <unistd.h>

class CGIHandler
{
	
	std::vector<std::string> vars;
	char **env;
	std::string script;
    std::string output;
	std::string body;
	public:
		CGIHandler(std::string _script) : script(_script), env(NULL) {};
		~CGIHandler();

		void setEnvVars(const Request& request, const std::string& script_path, const std::string &script_name , const t_server& server);
		void execute();
		std::string getOutput() const;
		void parseheader(Response &response);
		void parsebody(std::string output);

};