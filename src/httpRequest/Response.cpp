#include "Response.hpp"


Response::Response(/* args */)
{

}

Response::~Response()
{

}

void	Response::setStatusCode(err_codes &err){status = err;}

void	Response::setStatusCode(status_code &s){status = s;}

void	Response::setVersion(std::string &v){version = v;}

void	Response::setMessage()
{
	intstrMap messages;
	if (messages.empty())
	{
		messages[200] = "OK";
		messages[201] = "Created";
		messages[204] = "No Content";
		messages[301] = "Moved Permanently";
		messages[302] = "Found";
		messages[304] = "Not Modified";
		messages[400] = "Bad Request";
		messages[403] = "Forbidden";
		messages[404] = "Not Found";
		messages[405] = "Method Not Allowed";
		messages[408] = "Request Timeout";
		messages[411] = "Length Required";
		messages[413] = "Payload Too Large";
		messages[414] = "URI Too Long";
		messages[431] = "Header Field Too Large";
		messages[500] = "Internal Server Error";
		messages[501] = "Not Implemented";
		messages[505] = "HTTP Version Not Supported";
	}
	message = messages[status];
}

void	Response::setHeader(std::string &key, std::string &value){headers[key] = value;}

