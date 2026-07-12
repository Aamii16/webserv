#include "CGIHandler.hpp"
#include <cstdlib>
#include <cerrno>

CGIHandler::CGIHandler(std::string _script)
	: env(NULL), script(_script), body_offset(0), pid(-1), stdin_fd(-1), stdout_fd(-1)
{}

CGIHandler::~CGIHandler()
{
	for (size_t i = 0; i < vars.size(); ++i)
		delete[] env[i];
	delete[] env;
	if (stdin_fd >= 0)
		close(stdin_fd);
	if (stdout_fd >= 0)
		close(stdout_fd);
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
	target_path = script_path;
	vars.push_back("REQUEST_METHOD=" + request.getMethodString());
	vars.push_back("GATEWAY_INTERFACE=CGIHandler/1.1");
	vars.push_back("SERVER_NAME="+ server.server_name);
	// a t_server can listen on several host:port pairs; we don't know which
	// one this particular client connected through, so fall back to the
	// first configured port as a reasonable approximation.
	vars.push_back("SERVER_PORT=" + num_to_str(server.ports.empty() ? 0 : server.ports.front().first));
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

bool CGIHandler::start()
{
	int pipe_out[2]; 
	int pipe_in[2];  

	if (pipe(pipe_out) == -1)
		return false;
	if (pipe(pipe_in) == -1)
	{
		close(pipe_out[0]);
		close(pipe_out[1]);
		return false;
	}

	pid = fork();
	if (pid == -1)
	{
		close(pipe_out[0]); close(pipe_out[1]);
		close(pipe_in[0]);  close(pipe_in[1]);
		pid = -1;
		return false;
	}

	if (pid == 0)
	{
		close(pipe_in[1]);
		close(pipe_out[0]);

		if (dup2(pipe_in[0], STDIN_FILENO) == -1)
			_exit(1);
		if (dup2(pipe_out[1], STDOUT_FILENO) == -1)
			_exit(1);

		close(pipe_in[0]);
		close(pipe_out[1]);

		char *argv[] = { const_cast<char*>(script.c_str()), const_cast<char*>(target_path.c_str()), NULL };
		execve(script.c_str(), argv, env);
		_exit(1); 
	}

	close(pipe_in[0]);
	close(pipe_out[1]);

	stdin_fd  = pipe_in[1];
	stdout_fd = pipe_out[0];

\
	fcntl(stdin_fd,  F_SETFL, O_NONBLOCK);
	fcntl(stdout_fd, F_SETFL, O_NONBLOCK);

	body_offset = 0;
	if (body.empty())
	{
		close(stdin_fd);
		stdin_fd = -1;
	}
	return true;
}

bool CGIHandler::hasBodyToWrite() const
{
	return stdin_fd >= 0 && body_offset < body.size();
}

int CGIHandler::writeStdin()
{
	if (stdin_fd < 0 || body_offset >= body.size())
		return 0;

	ssize_t n = write(stdin_fd, body.c_str() + body_offset, body.size() - body_offset);
	if (n < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return -2;
		return -1; // caller removes the fd from epoll and calls closeStdin()
	}
	body_offset += static_cast<size_t>(n);
	return static_cast<int>(n);
}

int CGIHandler::readStdout()
{
	char buf[4096];
	ssize_t n = read(stdout_fd, buf, sizeof(buf));
	if (n > 0)
		output.append(buf, static_cast<size_t>(n));
	return static_cast<int>(n);
}

void CGIHandler::closeStdin()
{
	if (stdin_fd >= 0)
	{
		close(stdin_fd);
		stdin_fd = -1;
	}
}

void CGIHandler::closeStdout()
{
	if (stdout_fd >= 0)
	{
		close(stdout_fd);
		stdout_fd = -1;
	}
}

void CGIHandler::reap(int &exit_status)
{
	exit_status = 0;
	if (pid > 0)
	{
		waitpid(pid, &exit_status, 0);
		pid = -1;
	}
}

int   CGIHandler::getStdinFd()  const { return stdin_fd; }
int   CGIHandler::getStdoutFd() const { return stdout_fd; }
pid_t CGIHandler::getPid()      const { return pid; }

std::string CGIHandler::getOutput() const
{
	return output;
}

static bool findHeaderEnd(const std::string &s, size_t &pos, size_t &seplen)
{
	size_t p1 = s.find("\r\n\r\n");
	size_t p2 = s.find("\n\n");

	if (p1 != std::string::npos && (p2 == std::string::npos || p1 <= p2))
	{
		pos = p1;
		seplen = 4;
		return true;
	}
	if (p2 != std::string::npos)
	{
		pos = p2;
		seplen = 2;
		return true;
	}
	return false;
}

void CGIHandler::parseHeaders(Response &response)
{
	size_t pos, seplen;
	std::string headerPart = findHeaderEnd(output, pos, seplen) ? output.substr(0, pos) : output;
	int statusCode = 200;

	size_t start = 0;
	while (start < headerPart.size())
	{
		size_t eol = headerPart.find('\n', start);
		std::string line = (eol == std::string::npos) ? headerPart.substr(start) : headerPart.substr(start, eol - start);
		start = (eol == std::string::npos) ? headerPart.size() : eol + 1;

		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			continue;

		size_t colon = line.find(':');
		if (colon == std::string::npos)
			continue;

		std::string key = line.substr(0, colon);
		size_t vstart = line.find_first_not_of(" \t", colon + 1);
		std::string value = (vstart == std::string::npos) ? "" : line.substr(vstart);

		if (key == "Status")
			statusCode = std::atoi(value.c_str());
		else
			response.setHeader(key, value);
	}
	status_code sc = static_cast<status_code>(statusCode);
	response.setStatusCode(sc);
}

std::string CGIHandler::parseBody() const
{
	size_t pos, seplen;
	if (findHeaderEnd(output, pos, seplen))
		return output.substr(pos + seplen);
	return output;
}
