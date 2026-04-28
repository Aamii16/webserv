#include "httpResponse.hpp"


void	Request::parse_token_value(std::string &line, std::string &token, std::string &value, std::string delimiter)
{
	size_t pos = line.find(delimiter);
	if (pos == std::string::npos)
		throw BAD_REQUEST;
	token = line.substr(0, pos);
	if (pos + delimiter.size() > line.size())
		throw BAD_REQUEST;
	line = line.substr(pos + delimiter.size());
	value = line;
	if (token.empty() || value.empty())
		throw BAD_REQUEST;
	if (line.find(": ") != std::string::npos)
		throw BAD_REQUEST;

}

static void	valid_request_line(int	&method, std::string &target, std::string &version)
{
	if (!method)
		throw METHOD_NOT_ALLOWED;
	if (target.size() > MAX_URI_LENGTH)
		throw URI_TOO_LONG;
	if (version != "HTTP/1.1" && version != "HTTP/1.0")
		throw UNSUPPORTED_HTTP;
	
}

static int	valid_header(std::string &key, std::string &value)
{
	size_t idx;

	for(idx = 0; idx < key.size();++idx)
	{
		if (!isalnum(key[idx]) && key[idx] != '-')
			return 0;
	}
	for (idx = 0;idx < value.size();++idx)
	{
		if (value[idx] < 32 || value[idx] > 126)
			return 0;
	}
	if (key.size() > 256 || value.size() > MAX_HEADER_FIELD_LENGTH)
		throw HEADER_FIELD_TOO_LARGE;
	if (key == "Content-Length")
	{
		for(size_t i = 0;i < value.size(); ++i)
		{
			if (!isdigit(value[i]))
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
	valid_request_line(method, target, version);
	if ((pos = target.find("?")) != std::string::npos)
		parse_token_value(target, target, query, "?");
	state = HEADERS;
}

void	Request::parse_header(std::string header)
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
			std::cout << "REACHED BODY\n";
			state = BODY;
			break ;
		}
		if (state == REQ_LINE)
			parse_request_line(line);
		else if (valid_header(token, value))
		{
			parse_token_value(line, token, value, ": ");
			headers[token] = value;
		}
		if (line.find(" ") != std::string::npos)
			throw BAD_REQUEST;
		if (header.size() > pos + 2)
			header = header.substr(pos + 2);
		else
			break ;
	}
}


void	Request::parse_body()
{
	if(getContentLength() == 0 || method != POST || r_buffer.empty())
	{
		state = COMPLETE;
		return ;
	}
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
