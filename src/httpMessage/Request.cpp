#include "Request.hpp"
#include "webserv.h"

Request::Request(){err_code = 0;state = 0;method = 0;content_length = 0;}


int	Request::getMethod()const {return method;}

std::string	Request::getMethodString()const
{
	if (method == GET)
		return "GET";
	else if (method == POST)
		return "POST";
	else if (method == DELETE)
		return "DELETE";
	else
		return "";
}

int	Request::getState()const{return state;}

size_t	Request::getContentLength(){return content_length;}

void Request::setContentLength()
{
	strstrMap::const_iterator it = headers.find("Content-Length");
	if (it == headers.end())
		throw LENGTH_REQUIRED;
	else{
		for (size_t idx = 0; idx < it->second.size(); idx++)
			content_length = content_length * 10 + (it->second[idx] - '0');
	}
}

std::string		Request::getTarget()const {return target;}

std::string		Request::getVersion()const {return version;}

std::string		Request::getQuery()const {return query;}

std::string		Request::getBody()const {return body;}

strstrMap		Request::getHeaders()const{ return headers;}


std::string		Request::getHeaderValue(const std::string &key)const
{
	strstrMap::const_iterator it = headers.find(key);
	if (it != headers.end())
		return it->second;
	return "";
}
