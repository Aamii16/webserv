#include "Request.hpp"
#include "webserv.h"

Request::Request(){err_code = 0;state = 0;method = 0;content_length = 0;}

void	Request::parse_request(std::string buffer, const size_t &max_body_size)
{
	r_buffer += buffer;
	if (state == 0)
		state = REQ_LINE;
	if (state == REQ_LINE || state == HEADERS)
	{
		size_t pos = r_buffer.rfind("\r\n");
		if (pos != std::string::npos)
		{
			parse_header(r_buffer);
			if (r_buffer.size() > pos + 2)
				r_buffer = r_buffer.substr(pos + 2);
			else
				r_buffer = "";
		}
		else if (r_buffer.size() > MAX_HEADER_FIELD_LENGTH)
			throw HEADER_FIELD_TOO_LARGE;
	}
	if (state == BODY)
		parse_body(max_body_size);
}

int	Request::getMehod()const {return method;}

int	Request::getState()const{return state;}

size_t	Request::getContentLength(){return content_length;}

void Request::setContentLength()
{
	strstrMap::const_iterator it = headers.find("Content-Length");
	if (it == headers.end())
		content_length = 0;
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
