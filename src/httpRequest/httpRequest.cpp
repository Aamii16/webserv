#include "httpRequest.hpp"
#include "webserv.h"

Request::Request(){state=0;}

void	Request::parse_token_value(std::string &line, std::string &token, std::string &value, std::string delimiter)
{
	size_t pos = line.find(delimiter);
	// if (pos == std::string::npos)
	// 	throw RequestError
	token = line.substr(0, pos);
	value = line.substr(pos + delimiter.size(), std::string::npos);
	// 	throw RequestError if token or val is empty
	// std::cout << "curr line: " << line << "\ncurr token: " << token << "\ncurr val: " << value << "--\n";
}

void	Request::parse_request_line(std::string &line)
{
	std::string	token;
	parse_token_value(line, token, line, " ");
	parse_token_value(line, target, version, " ");
	if (token == "GET")
		method = GET;
	else if (token == "POST")
		method = POST;
	else if (token == "DELETE")
		method = DELETE;
}

void	Request::parse_header(std::string header)
{
	std::string	token, value, line;
	size_t	pos = 0;
	pos = header.find("\r\n");

	//throw error if pos == std::string::npos
	line = header.substr(0, pos);
	parse_request_line(line);
	while (!header.empty())
	{
		header = header.substr(pos + 2, std::string::npos);
		pos = header.find("\r\n");
		// if (pos == std::string::npos)
			// throw error ;
		if (!pos)
			break ;
		line = header.substr(0, pos);
		if (line.empty())
			break ;
		parse_token_value(line, token , value, ": ");
		headers[token] = value;
	}
}

void	Request::parse_request(std::string buffer)
{
	size_t	pos = 0;
	r_buffer += buffer;
	// std::cout << r_buffer << "---\n";
	if (state == 0)
		state = HEADER;
	if (state == HEADER)	
		pos = r_buffer.find("\r\n\r\n");
	if (state == HEADER && pos != std::string::npos)
	{
		parse_header(r_buffer.substr(0, pos + 2));
		state = BODY;
		r_buffer = r_buffer.substr(pos + 4, std::string::npos);
	}
	if (state == BODY)
	{
		size_t	length = std::strtol(headers["content_length"].c_str(), NULL, 10);
		size_t idx;
		std::cout << "vody\n";
		for (idx = 0;idx < buffer.size() && body.size() != length;idx++)
			body += r_buffer[idx];
		if (body.size() == length || r_buffer.empty())
		{
			state = COMPLETE;
			r_buffer = r_buffer.substr(idx, std::string::npos);
		}
		else
			r_buffer = "";
		//ADD CHECKS FOR CLIENT_MAX_BODY_SIZE
	}
}


int   			Request::getMehod()const
{
	return method;
}

std::string		Request::getTarget()const
{
	return target;
}

std::string		Request::getVersion()const
{
	return version;
}

std::string		Request::getBody()const
{
	return body;
}

strstrMap		Request::getHeaders()const{
	return headers;
}

std::string		Request::getHeaderValue(std::string &key)const
{
	strstrMap::const_iterator it = headers.find(key);
	if (it != headers.end())
		return it->second;
	else
		return "";
	// throw error i guess;
}
