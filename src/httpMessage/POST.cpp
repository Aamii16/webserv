#include "Handler.hpp"


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

std::string  mkfile(unsigned int file_counter, const std::string &extension)
{
	std::string prefix = "uploaded_";
	std::string name;
	std::string postfix;
	std::stringstream ss;

	// This is to handle the case when the counter reaches its maximum value
	// and wraps around to 0, which could lead to overwriting existing files. By appending underscores to the prefix, 
	//we can create a new namespace for the file names, allowing us to continue generating unique file names even after
	// the counter resets.
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
	
	// i don't like this file_counter appraoch , it's not very efficient
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
	if (stat(loc.upload_path.c_str(), &st) == -1) {
	    if (errno == ENOENT)
	        throw NOT_FOUND;
	    else
	        throw INTERNAL_SERVER_ERROR;
	}
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

