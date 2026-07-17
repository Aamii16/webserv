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

const location	&Handler::findLocation(t_server &server, std::string &path)
{
	strlocationMap::const_iterator it = server.locations.begin();
	std::string target = request.getTarget();

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
	path = (!it->second.root.empty() ? it->second.root : server.root) + "/" + target.substr(it->first.size());
	if (path.empty())
		throw NOT_FOUND;
	return it->second;
}
