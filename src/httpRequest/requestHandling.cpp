#include "Request.hpp"
#include "webserv.h"

const std::string get_extension(const std::string &content_type)
{
	static strstrMap types;
	if (types.empty())
	{
		types["text/html"] = "html";
		types["text/css"] = "css";
		types["application/javascript"] = "js";
		types["application/json"] = "json";
		types["text/plain"] = "txt";
		types["application/xml"] = "xml";
		types["image/jpg"] = "jpg";
		types["image/png"] = "png";
		types["image/gif"] = "gif";
		types["image/svg+xml"] = "svg";
		types["image/vnd.microsoft.icon"] = "ico";
		types["image/bmp"] = "bmp";
		types["image/webp"] = "webp";
		types["video/mp4"] = "mp4";
		types["video/mpeg"] = "mpeg";
		types["audio/mpeg"] = "mp3";
		types["audio/wav"] = "wav";
		types["audio/ogg"] = "ogg";
		types["application/pdf"] = "pdf";
		types["application/zip"] = "zip";
		types["application/x-tar"] = "tar";
		types["application/gzip"] = "gz";
		types["application/x-7z-compressed"] = "7z";
		types["text/csv"] = "csv";
		types["font/woff"] = "woff";
		types["font/woff2"] = "woff2";
		types["font/ttf"] = "ttf";
	}
	strstrMap::const_iterator it = types.find(content_type);
	if (it != types.end())
		return ("." + it->second);
	return "";
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


void update_counter(const char *name, unsigned int &count, int flag) // flag is used to determine whether to read/update counter from file, or t o write update counter on the file
{
	std::fstream file;

	if (flag == 'r')
	{
		file.open(name, std::ios::in);
		if (file.is_open())
		{
			file >> count;
			file.close();
		}
		else
			count = 0;
	}
	else if (flag == 'w')
	{
		file.open(name, std::ios::out | std::ios::trunc);
		if (file.is_open())
		{
			file << count;
			file.close();
		}
	}
}

std::string  mkfile(const std::string &extension)
{
	static unsigned int count;
	static std::string prefix = "uploaded_";
	std::string name;
	
	update_counter("upload_counter", count, 'r');
	// This is to handle the case when the counter reaches its maximum value and wraps around to 0, which could lead to overwriting existing files. By appending underscores to the prefix, we can create a new namespace for the file names, allowing us to continue generating unique file names even after the counter resets.
	// if (count == UINT_MAX)
	// {
	// 	count = 0;
	// 	static unsigned int idx;
	// 	update_counter("upload_idxcounter", count, 'r');		
	// 	for (unsigned int i = 0; i < idx; ++i)	
	// 		prefix += "_";
	// 	idx++;
	// 	update_counter("upload_idxcounter", count, 'w');
	// }	might just delete this
	std::string postfix;
	std::stringstream ss;
	ss << count++;
	ss >> postfix;
	name = prefix + postfix + get_extension(extension);
	update_counter("upload_counter", count, 'w');
	return name;
}

void	handle_post(const Request &req, const std::string &resolvedpath, const location &loc)
{
	std::string name;
	struct stat st;

	if (loc.upload_path.empty())
		throw FORBIDDEN;
	if (stat(loc.upload_path.c_str(), &st) == -1)
		throw INTERNAL_SERVER_ERROR;
	if (!S_ISDIR(st.st_mode))
		throw FORBIDDEN;
	errno = 0;
	name = mkfile(req.getHeaderValue("Content-Type"));
	int fd = open((loc.upload_path + "/" + name).c_str(), O_WRONLY | O_CREAT, 0644);
	if (fd == -1 && errno == EACCES)
		throw FORBIDDEN;
	else if (fd == -1 && errno == ENOSPC)
		throw INSUFFICIENT_STORAGE;
	else if (fd == -1)
		throw INTERNAL_SERVER_ERROR;
	write(fd, req.getBody().c_str(), req.getBody().size());
	close(fd);
	throw CREATED;
}


void     Request::handle_request(const t_server &server)
{
	strlocationMap::const_iterator it = server.locations.begin();
	if (headers.find("Host") == headers.end())
		throw BAD_REQUEST;
	if (method == POST)
		target += "/";
	normalise_target(target);
	for (;it != server.locations.end();++it){
		if (target.size() >= it->first.size() && !target.compare(0, it->first.size(), it->first) &&
			(target.size() == it->first.size() || target[it->first.size()] == '/'))
			break;
	}
	if (it == server.locations.end())
		it = server.locations.find("/");
	if (it == server.locations.end())
		throw NOT_FOUND;
	// if (!it->second.return_directive.empty())
	// 	redirection();
	std::string path = (!it->second.root.empty() ? it->second.root : server.root) + "/" + target.substr(it->first.size());
	switch (method)
	{
		case GET:
			// handle_get(path);
			break;
		case POST:
			handle_post(*this, path, it->second);
			break;
		case DELETE:
			// handle_delete(path);
			break;
		default:
			throw NOT_IMPLEMENTED;
	}
}
