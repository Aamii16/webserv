#include "webserv.h"


std::string	num_to_str(long long num)
{
	std::stringstream ss;
	ss << num;
	std::string num_str;
	ss >> num_str;
	return num_str;
}


const strstrMap &mimeTypes()
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
	return types;
}

void	p_error(std::string err)
{
	std::cerr << err << std::endl;
}