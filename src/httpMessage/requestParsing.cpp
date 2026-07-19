#include "Response.hpp"
#include "webserv.h"

void	Request::parse_token_value(std::string &line, std::string &token, std::string &value, std::string delimiter)
{
	size_t pos = line.find(delimiter);
	if (pos == std::string::npos){

		std::cout << "parsing token value at state " << state << "in: "<< line << std::endl;
		throw BAD_REQUEST;
	}
	std::string tmp_token = line.substr(0, pos);
	if (pos + delimiter.size() > line.size())
		throw BAD_REQUEST;
	value = line.substr(pos + delimiter.size());
	token = tmp_token;
}

int	has_whitespace(std::string &str)
{
	for (size_t i = 0; i < str.size();++i)
	{
		if (isspace(str[i]))
			return 1;
	}
	return 0;
}

static void	valid_request_line(int	&method, std::string &target, std::string &version)
{
	if (has_whitespace(target) || target.empty() || version.empty() || target[0] != '/')
		throw BAD_REQUEST;
	if (!method)
		throw METHOD_NOT_ALLOWED;
	if (target.size() > MAX_URI_LENGTH)
		throw URI_TOO_LONG;
	// might wanna change this to 1.1 only
	if (version != "HTTP/1.1" && version != "HTTP/1.0")
		throw UNSUPPORTED_HTTP;
}


static int	valid_header(std::string &key, std::string &value)
{
	size_t idx;

	for(idx = 0; idx < key.size();++idx)
	{
		if (!isalnum(key[idx]) && key[idx] != '-')
			throw BAD_REQUEST;
	}
	for (idx = 0;idx < value.size();++idx)
	{
		if (value[idx] < 32 || value[idx] > 126)
			throw BAD_REQUEST;
	}
	if (key.size() > 256 || value.size() > MAX_HEADER_FIELD_LENGTH)
		throw HEADER_FIELD_TOO_LARGE;
	if (key == "Content-Length")
	{
		for(size_t i = 0;i < value.size(); ++i)
		{
			if (!isdigit(static_cast<unsigned char>(value[i])))
				throw BAD_REQUEST;
		}
		
	}
	return 1;
}

void	Request::parse_request_line(std::string &line)
{
	std::string	token;
	size_t pos;

	parse_token_value(line, token, line, " ");
	parse_token_value(line, target, version, " ");
	if (token == "GET")
		method = GET;
	else if (token == "POST")
		method = POST;
	else if (token == "DELETE")
		method = DELETE;
	else
		throw NOT_IMPLEMENTED;
	valid_request_line(method, target, version);
	if ((pos = target.find("?")) != std::string::npos)
		parse_token_value(target, target, query, "?");
	state = HEADERS;
}

void	Request::parse_header(std::string &header)
{
	std::string	token, value, line;
	size_t	pos = 0;

	while ((state == HEADERS || state == REQ_LINE) && !header.empty())
	{
		pos = header.find("\r\n");
		if (pos == std::string::npos)
			break ;
		line = header.substr(0, pos);
		if (line.empty())
		{
			state = BODY;
			if (header.size() > pos + 2)
				header = header.substr(pos + 2);
			else
				header = "";
			break ;
		}	
		if (state == REQ_LINE)
			parse_request_line(line);
		else if (valid_header(token, value))
		{
			parse_token_value(line, token, value, ": ");
			if (token.empty() || has_whitespace(token) || !headers[token].empty())
				throw BAD_REQUEST;
			headers[token] = value;
		}
		if (header.size() > pos + 2)
			header = header.substr(pos + 2);
		else
		{
			header = "";
			break ;
		}
	}
}


void	Request::parse_body(const size_t &max_body_size)
{
	if(method != POST || r_buffer.empty())
	{
		state = COMPLETE;
		return ;
	}
	if (headers.find("Content-Type") == headers.end())
		throw BAD_REQUEST;
	setContentLength();
	if (!content_length)
	{
		state = COMPLETE;
		return ;
	}
	if (content_length > max_body_size)
		throw PAYLOAD_TOO_LARGE;
	size_t	idx;
	for (idx = 0; idx < r_buffer.size(); idx++)
	{
		body += r_buffer[idx];
		if (body.size() == content_length)
		{
			state = COMPLETE;
			r_buffer = r_buffer.substr(idx);
			return ;
		}
	}
	if (body.size() != content_length)
		r_buffer = "";
}

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
			//now for some reason it stays stuck in same line (first line)
			parse_header(r_buffer);
			// turns out im substringint twice (first in parse_header)
			// this could be troublesome later
			// now i realised i actually need to substring here, but i don't remember why i commented it before
			// pos = r_buffer.find("\r\n");
			// if (pos != std::string::npos && r_buffer.size() > pos + 2)
				// r_buffer = r_buffer.substr(pos + 2);
			// else
				// r_buffer = "";
		}
		else if (r_buffer.size() > MAX_HEADER_FIELD_LENGTH)
			throw HEADER_FIELD_TOO_LARGE;
	}
	if (state == BODY)
		parse_body(max_body_size);
}