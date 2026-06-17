#include "Request.hpp"
#include "webserv.h"

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

std::string	handle_dir(const location &loc, std::string &path)
{
	if (!loc.auto_idx)
		throw FORBIDDEN;

	std::string listings;
	std::string body;
	DIR *dir = opendir(path.c_str());

	if (!dir) {
	    if (errno == EACCES)
	        throw FORBIDDEN;
		else
	        throw INTERNAL_SERVER_ERROR;
	}
	struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..")
			listings += "<li><a href=\"" + loc.alias + "/" + entry->d_name +"\">"+entry->d_name+"</a></li>";
	}
    closedir(dir);
	body = 
		"<!DOCTYPE html>"
    	"<html lang=\"en\">"
    	"<head>"
    	"<meta charset=\"UTF-8\" />"
    	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />"
    	"<script src=\"https://cdn.tailwindcss.com\"></script>"
    	"<title>autoindex</title>"
    	"</head>"
    	"<body class=\"bg-gray-200 text-center\">"
    	"<div class=\"mx-auto min-h-screen max-w-3xl bg-blue-100 py-10 border-l-4 border-r-4 border-black px-6\">"
    	"<h1 class=\"font-bold text-8xl\">Index of</h1>"
    	"<h1 class=\"font-bold text-5xl my-8\">" + loc.alias + 
    	"</h1>"
    	"<div class=\"flex flex-col bg-white max-w-xl mx-auto border-2 border-black rounded-2xl\">"
    	"<ul class=\"list-none\">" + listings + 
		"</ul>"
    	"</div>"
    	"</div>"
    	"</body>"
    	"</html>";
	return body;
}

void	Handler::handle_get(const location &loc, std::string &path)
{
	struct stat st;
	std::string ressource;

	if (stat(path.c_str(), &st) == -1)
		throw INTERNAL_SERVER_ERROR;
	if (S_ISDIR(st.st_mode))
	{
		if (loc.index.empty())
			ressource = handle_dir(loc, path);
		else
		{
			path += "/" + loc.index;
			if (stat(path.c_str(), &st) == -1)
				throw INTERNAL_SERVER_ERROR;
		}
	}
	else if (S_ISREG(st.st_mode) || !loc.index.empty()){
		errno = 0;
		int fd = open(path.c_str(), O_RDONLY);
		if (fd == -1 && errno == EACCES)
			throw FORBIDDEN;
		else if (fd == -1)
			throw INTERNAL_SERVER_ERROR;
		char buf[1024];
		ssize_t bytes_read;
		while ((bytes_read = read(fd, buf, sizeof(buf))) > 0)
			ressource.append(buf, bytes_read);
		if (bytes_read == -1)
		{
			close(fd);
			throw INTERNAL_SERVER_ERROR;
		}
		close(fd);
	}
	response.setBody(ressource);
	throw OK;
}

void     Handler::handle_request(t_server &server)
{
	strlocationMap::const_iterator it = server.locations.begin();
	std::string target = request.getTarget();

	if (request.getHeaderValue("Host").empty())
		throw BAD_REQUEST;
	if (request.getMethod() == POST)
		request.getTarget() += "/";
	normalise_target(target);
	request.setTarget(target);
	for (;it != server.locations.end();++it){
		if (target.size() >= it->first.size() && !target.compare(0, it->first.size(), it->first) &&
			(target.size() == it->first.size() || target[it->first.size()] == '/'))
			break;
	}
	if (it == server.locations.end())
		it = server.locations.find("/");
	if (it == server.locations.end())
		throw NOT_FOUND;
	if (!it->second.redirection.second.empty())
			response.redirect(it->second.redirection);
	if (!it->second.methods.at(request.getMethod()))
		throw METHOD_NOT_ALLOWED;
	std::string path = (!it->second.root.empty() ? it->second.root : server.root) + "/" + target.substr(it->first.size());
	switch (request.getMethod())
	{
		case GET:
			handle_get(it->second , path);
			break;
		case POST:
			handle_post(it->second, server.upload_counter);
			break;
		case DELETE:
			// handle_delete(path);
			break;
		default:
			throw NOT_IMPLEMENTED;
	}
}
