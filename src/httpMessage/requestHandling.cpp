#include "Request.hpp"
#include "webserv.h"
#include "CGIHandler.hpp"



void Handler::handle_delete(std::string &path)
{
	struct stat st;

	errno  = 0;
	validate_file_path(stat(path.c_str(), &st));
	if (S_ISDIR(st.st_mode))
		throw FORBIDDEN;
	errno = 0;
	validate_file_path(unlink(path.c_str()));
	throw NO_CONTENT;
}

void	Handler::handle_cgi(const location &loc, std::string &path, const t_server& server)
{
	struct stat st;
	std::string	ext;

	size_t pos = path.rfind(".");
	if (pos == std::string::npos)
		throw FORBIDDEN;
	errno  = 0;
	validate_file_path(stat(path.c_str(), &st));
	if (S_ISDIR(st.st_mode))
		throw FORBIDDEN;
	ext = path.substr(pos);
	if (loc.cgi_ext.find(ext) == loc.cgi_ext.end())
		throw FORBIDDEN;

	cgi = new CGIHandler(loc.cgi_ext.at(ext));
	cgi->setEnvVars(request, path, request.getTarget(), server);
	if (!cgi->start())
	{
		delete cgi;
		cgi = NULL;
		throw INTERNAL_SERVER_ERROR;
	}
	state = CGI_WAIT;
}

void     Handler::handle_request(t_server &server)
{
	std::string path;
	if (request.getHeaderValue("Host").empty())
		throw BAD_REQUEST;
	const location loc = findLocation(server, path);
	if (loc.cgi)
	{
		handle_cgi(loc, path, server);
		return ;
	}
	switch (request.getMethod())
	{
		case GET:
			handle_get(loc , path);
			break;
		case POST:
			handle_post(loc, server.upload_counter);
			break;
		case DELETE:
			handle_delete(path);
			break;
		default:
			throw NOT_IMPLEMENTED;
	}
}
