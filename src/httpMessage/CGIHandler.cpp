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
	body = request.getBody();
}

void CGIHandler::execute()//const std::string &body
{
    int pipe_out[2];
    int pipe_in[2];

    if (pipe(pipe_out) == -1 || pipe(pipe_in) == -1)
        throw INTERNAL_SERVER_ERROR;

    pid_t pid = fork();
    if (pid == -1)
        throw INTERNAL_SERVER_ERROR;

    if (pid == 0)
    {
        close(pipe_in[1]);
        close(pipe_out[0]);

        if (dup2(pipe_in[0], STDIN_FILENO) == -1)  exit(1);
        if (dup2(pipe_out[1], STDOUT_FILENO) == -1) exit(1);

        close(pipe_in[0]);
        close(pipe_out[1]);

        char *argv[] = { (char *)script.c_str(), NULL };
        execve(script.c_str(), argv, env);
        exit(1); // execve failed
    }

    close(pipe_in[0]);
    close(pipe_out[1]);

    // POST body to child stdin
    if (!body.empty())
        write(pipe_in[1], body.c_str(), body.size());
    close(pipe_in[1]);

    // Read CGI output
    char buf[5000];
    ssize_t n;
    while ((n = read(pipe_out[0], buf, sizeof(buf))) > 0)
        _output.append(buf, n);
    close(pipe_out[0]);

    int status;
    waitpid(pid, &status, 0);

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        throw INTERNAL_SERVER_ERROR;
}

void CGIHandler::parseheader(Response &response)
{

}

void CGIHandler::parsebody(std::string output)
{

}
std::string CGIHandler::getOutput() const
{
    return output;
}
