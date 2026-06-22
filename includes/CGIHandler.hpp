#pragma once
#include "Request.hpp"

class CGIHandler
{
	
	std::vector<std::string> vars;
	char **env;
	std::string script;private:
    std::string output;
	std::string body;
	public:
		CGIHandler(std::string _script) : script(_script), env(NULL) {};
		~CGIHandler();

		void setEnvVars(const Request& request, const std::string& script_path, const std::string &script_name , const t_server& server);
		void execute();
		std::string getOutput() const;
		void CGIHandler::parseheader(Response &response);
		void CGIHandler::parsebody(std::string output);

};