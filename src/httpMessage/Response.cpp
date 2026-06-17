#include "Response.hpp"


Response::Response(/* args */)
{

}

Response::Response(int &code)
{
	status = code;
	setMessage();
}

Response::~Response()
{

}

void	Response::setStatusCode(err_codes &err){status = err;setMessage();}

void	Response::setStatusCode(status_code &s){status = s;setMessage();}

void	Response::setVersion(const std::string &v){version = v;}

std::string getCodeMessage(int &code)
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
		// Error codes
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
	return messages[code];
}

void	Response::setMessage()
{
	
	message = getCodeMessage(status);
};

void Response::print_response()const
{
	std::cout << "=== HTTP RESPONSE ===" << std::endl;
	std::cout << "Status: " << status << " " << message << std::endl;
	std::cout << "Version: " << version << std::endl;
	std::cout << "--- Headers ---" << std::endl;
	strstrMap tmp = headers;
	for (strstrMap::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
		std::cout << it->first << ":" << it->second << std::endl;
	
	std::cout << "\n--- Body ---" << std::endl;
	std::cout << body << std::endl;
}

void	Response::setHeader(std::string key, std::string value){headers[key] = value;}
std::string Response::getHeader(const std::string &key)const{
	strstrMap::const_iterator it = headers.find(key);
	if (it != headers.end())
		return it->second;
	return "";
}


void	Response::redirect(const intstrPair &redir)
{
	setHeader("Location", redir.second);
	throw static_cast<status_code>(redir.first);
}

std::string	Response::mkResponse()
{
	strstrMap::const_iterator it = headers.begin();
	response_buffer = version + " " + std::to_string(status) + " " + message + "\r\n";
	while (it != headers.end())
	{
		response_buffer += it->first + ": " + it->second + "\r\n";
		it++;
	}
	response_buffer += "\r\n" + body;
	return response_buffer;
}