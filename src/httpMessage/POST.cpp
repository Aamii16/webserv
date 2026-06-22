#include "Handler.hpp"

const std::string get_extension(const std::string &content_type)
{
	static strstrMap types = mimeTypes();
	
	strstrMap::const_iterator it = types.find(content_type);
	if (it != types.end())
		return ("." + it->second);
	return "";
}

std::string  mkfile(unsigned int file_counter, const std::string &extension)
{
	std::string prefix = "uploaded_";
	std::string name;
	std::string postfix;
	std::stringstream ss;

	ss << file_counter++;
	ss >> postfix;
	name = prefix + postfix + get_extension(extension);
	return name;
}

void	Handler::handle_post(const location &loc, unsigned int &upload_counter)
{
	std::string name;
	struct stat st;

	if (loc.upload_path.empty())
		throw FORBIDDEN;
	errno  = 0;
	validate_file_path(stat(loc.upload_path.c_str(), &st));
	if (!S_ISDIR(st.st_mode))
		throw FORBIDDEN;
	errno = 0;
	name = mkfile(upload_counter, request.getHeaderValue("Content-Type"));
	int fd = open((loc.upload_path + "/" + name).c_str(), O_WRONLY | O_CREAT, 0644);
	if (fd == -1 && errno == EACCES)
		throw FORBIDDEN;
	else if (fd == -1 && errno == ENOSPC)
		throw INSUFFICIENT_STORAGE;
	else if (fd == -1)
		throw INTERNAL_SERVER_ERROR;
	upload_counter++;
	if (write(fd, request.getBody().c_str(), request.getBody().size()) == -1){
	    close(fd);
	    throw INTERNAL_SERVER_ERROR;
	}
	close(fd);
	std::string location_header;
	if (loc.alias[loc.alias.size() - 1] == '/')
		location_header = loc.alias + name;
	else
		location_header = loc.alias + "/" + name;
	response.setHeader("Location", location_header);	
	throw CREATED;
}

