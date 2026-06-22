#include "Request.hpp"
#include "webserv.h"
#include "CGIHandler.hpp"

void validate_file_path(int val){
    if (val == -1) {
        switch (errno) {
            case ENOENT:
                throw NOT_FOUND;
            case EACCES:
                throw FORBIDDEN;
            default:
                throw INTERNAL_SERVER_ERROR;
        }
    }
}

void	normalise_target(std::string &target)
{
	size_t	pos;

	while ((pos = target.find("/../")) != std::string::npos)
	{
		size_t last = 0;
		if (pos != 0)
			last = target.rfind('/', pos - 1);
		target.erase(last, 3 + pos - last);
	}
	while ((pos = target.find("/./")) != std::string::npos)
		target.erase(pos, 2);
	while ((pos = target.find("//")) != std::string::npos)
		target.erase(pos, 1);
}


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
	ext = path.substr(pos + 1);
	if (loc.cgi_ext.find(ext) == loc.cgi_ext.end())
		throw FORBIDDEN;
	CGIHandler cgi(loc.cgi_ext.at(ext));
	cgi.setEnvVars(request, request.getTarget(), path, server);
	cgi.execute();
}

void     Handler::handle_request(t_server &server)
{
	strlocationMap::const_iterator it = server.locations.begin();
	std::string target = request.getTarget();

	if (request.getHeaderValue("Host").empty())
		throw BAD_REQUEST;
	{
		if (request.getMethod() == POST)
			request.getTarget() += "/";
		normalise_target(target);
		request.setTarget(target);
		for (;it != server.locations.end();++it){
			if (target.size() >= it->first.size() && !target.compare(0, it->first.size(), it->first) &&
				(target.size() == it->first.size() || target[it->first.size()] == '/'))
				break;
		}
	}
	{
		if (it == server.locations.end())
			it = server.locations.find("/");
		if (it == server.locations.end())
			throw NOT_FOUND;
		if (!it->second.redirection.second.empty())
				response.redirect(it->second.redirection);
		if (!it->second.methods.at(request.getMethod()))
			throw METHOD_NOT_ALLOWED;
	}
	std::string path = (!it->second.root.empty() ? it->second.root : server.root) + "/" + target.substr(it->first.size());
	if (path.empty())
		throw NOT_FOUND;
	if (it->second.cgi)
		handle_cgi(it->second, path, server);
	switch (request.getMethod())
	{
		case GET:
			handle_get(it->second , path);
			break;
		case POST:
			handle_post(it->second, server.upload_counter);
			break;
		case DELETE:
			handle_delete(path);
			break;
		default:
			throw NOT_IMPLEMENTED;
	}
}
