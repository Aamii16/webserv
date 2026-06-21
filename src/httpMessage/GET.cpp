#include "Handler.hpp"

std::string	handle_dir(const location &loc, std::string &path)
{
	if (!loc.auto_idx)
		throw FORBIDDEN;

	std::string listings;
	std::string body;
	DIR *dir = opendir(path.c_str());

	if (!dir && errno == EACCES)
	        throw FORBIDDEN;
	else if (!dir && errno == ENOENT)
	     throw NOT_FOUND;
	else if (!dir)
	     throw INTERNAL_SERVER_ERROR;
	struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..")
			listings += "<li><a href=\"" + loc.alias + "/" + entry->d_name +"\">"+entry->d_name+"</a></li>";
	}
    closedir(dir);
	body = 
	    "<!DOCTYPE html>\n"
	    "<html>\n"
	    "<head>\n"
	    "    <meta charset=\"utf-8\">\n"
	    "    <title>Index</title>\n"
	    "    <style>\n"
	    "        body { font-family: monospace; padding: 20px; font-size: 14px; }\n"
	    "        ul { list-style: none; padding: 0; }\n"
	    "        li { padding: 6px 0; border-bottom: 1px solid #eee; }\n"
	    "        a { text-decoration: none; color: #0066cc; }\n"
	    "        a:hover { text-decoration: underline; }\n"
	    "    </style>\n"
	    "</head>\n"
	    "<body>\n"
	    "<h2>Index of /</h2>\n"
	    "<ul>\n"
	    "    <li><a href=\"../\">📁 .. " + loc.alias + "</a></li>" 
	    + listings + 
	    "</ul>\n"
	    "</body>\n"
	    "</html>";

	return body;
}

std::string get_mime_type(const std::string &path)
{
	std::string ext;
	size_t pos = path.rfind(".");
	if (pos == std::string::npos)
		return "text/plain";
	else
		ext = path.substr(pos + 1);
	strstrMap types = mimeTypes();
	for (strstrMap::const_iterator it = types.begin(); it != types.end(); ++it)
	{
		if (it->second == ext)
			return it->first;
	}
	return "text/plain";
}

void	Handler::handle_get(const location &loc, std::string &path)
{
	struct stat st;
	std::string ressource;

	validate_file_path(stat(path.c_str(), &st));
	if (S_ISDIR(st.st_mode))
	{
		if (loc.index.empty())
		{
			ressource = handle_dir(loc, path);
			response.setHeader("Content-Type", "text/html");
		}
		else
		{
			if (path[path.size() - 1] != '/')
				path += "/";
			path += loc.index;
			errno = 0;
			validate_file_path(stat(path.c_str(), &st));
		}
	}
	if (!loc.index.empty() || S_ISREG(st.st_mode)){
		errno = 0;
		int fd = open(path.c_str(), O_RDONLY);
		validate_file_path(fd);
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
		response.setHeader("Content-Type", get_mime_type(path));
	}
	response.setBody(ressource);
	throw OK;
}
