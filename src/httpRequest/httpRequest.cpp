#include "httpRequest.hpp"
#include "webserv.h"

Request::Request(){err_code = 0;state = 0;method = 0;content_length = 0;}

void	Request::parse_request(std::string buffer)
{
	try{
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
		}
		if (state == BODY)
			parse_body();
	}
	catch(codes &error){
		err_code = error;
	}
	catch(s_state &s){
		err_code = 400;
	}

}

int     Request::process(t_configuration &conf)
{
	
	if (headers.find("host") == headers.end())
		send_response(400); // Bad Request
	if (method == POST && headers.find("Content-Length") == headers.end())
		send_response(LENGTH_REQUIRED);
	if (body.size() > conf.server.max_body_size)
		send_response(413) // Playload too large ( body exceeds max_body_size)

	return 1;

}



// getters
int   			Request::getMehod()const {return method;}

int				Request::getState()const{return state;}

size_t				Request::getContentLength()
{
	if (content_length)
		return content_length;
	strstrMap::const_iterator it = headers.find("Content-Length");
	if (it == headers.end())
		return 0;
	long len = std::strtol(it->second.c_str(), NULL, 10);
	content_length = (size_t)len;
	return content_length;
}

std::string		Request::getTarget()const {return target;}

std::string		Request::getVersion()const {return version;}

std::string		Request::getQuery()const {return query;}

std::string		Request::getBody()const {return body;}

strstrMap		Request::getHeaders()const{ return headers;}


std::string		Request::getHeaderValue(std::string &key)const
{
	strstrMap::const_iterator it = headers.find(key);
	if (it != headers.end())
		return it->second;
	return "";
}
