#include "Connection.hpp"

//
void print_request(const Request &req)
{
    std::cout << "=== HTTP REQUEST ===" << std::endl;
	std::string method_str;
	if (req.getMethod() == GET)
		method_str = "GET";
	else if (req.getMethod() == POST)
		method_str = "POST";
	else if (req.getMethod() == DELETE)
		method_str = "DELETE";
	// etc
	std::cout << "Method:" << method_str << std::endl;
    std::cout << "Target:" << req.getTarget() << std::endl;
    std::cout << "Version:" << req.getVersion() << std::endl;
    std::cout << "--- Headers ---" << std::endl;
	strstrMap tmp = req.getHeaders();
	for (strstrMap::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
        std::cout << it->first << ":" << it->second << std::endl;
    
    std::cout << "\n--- Body ---" << std::endl;
    std::cout << req.getBody() << std::endl;
	std::cout << "state: " << req.getState() << std::endl;
}

Connection::Connection(int fd): fd(fd){}

std::string errpage(int code)
{
	std::string default_page = (
		"<!DOCTYPE html>"
		"<html lang=\"en\">"
		"<head>"
		"<meta charset=\"UTF-8\" />"
		"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />"
		"<script src=\"https://cdn.tailwindcss.com\"></script>"
		"<title>Error</title>"
		"</head>"
		"<body class=\"bg-gray-200 text-center\">"
		"<div class=\"mx-auto min-h-screen max-w-3xl bg-blue-100 py-10 border-l-4 border-r-4 border-black px-6\">"
		"<h1 class=\"font-bold text-8xl\">") + num_to_str(code) + std::string("</h1>"
		"<h1 class=\"font-bold text-5xl my-8\">") + getCodeMessage(code) + std::string(
		"</h1></div></body></html>"
	);
	return default_page;
}

std::string geterrpage(const t_server &server, int code)
{
	intstrMap::const_iterator it = server.err_pages.find(code);

	if (it != server.err_pages.end())
	{
	std::ifstream file(it->second.c_str());
		if (file.is_open())
		{
			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			file.close();
			return content;
		}		
	}
	return errpage(code);
}

void mkDate(std::string &date)
{
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	char buf[100];
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm);
	date = buf;
}


void Connection::setResponseHeaders(const t_server &server)
{
	std::string date;

	response.setVersion(request.getVersion());
	response.setMessage();
	
	if (!response.getBody().empty())
		response.setHeader("Content-Type", request.getHeaderValue("Content-Type"));
	response.setHeader("Content-Length", num_to_str(response.getBody().size()));
	if (!request.getHeaderValue("Connection").empty())
		response.setHeader("Connection", request.getHeaderValue("Connection"));
	else if (request.getVersion() == "HTTP/1.1")
		response.setHeader("Connection", "keep-alive");
	else
		response.setHeader("Connection", "close");
	// close connectioon if status >= 400 , nt sure about this
	mkDate(date);
	response.setHeader("Date", date);
	// i can't set server header cause i deleted server-name frm config and im too lazy t add it back
}


void Connection::process(t_server &server, std::string buffer)
{
	try{
		request.parse_request(buffer, server.max_body_size);
		if (request.getState() == COMPLETE)
			handle_request(server);
		else
			return ;
	}
	catch(err_codes &error){
		response.setStatusCode(error);
		response.setHeader("Content-Type", "text/html");
		std::cout << "Error Code: " << response.getMessage() << std::endl;
		// intstrMap::const_iterator it = server.err_pages.find(error);
		// if (it != server.err_pages.end())
		// 	response.setbody(it->second);
		// else
		// 	response.setbody(errpage(error));
	}
	catch(status_code &status){
		response.setStatusCode(status);
		std::cout << "status code: " << response.getMessage() << std::endl;
	}
	std::cout << "=== bitch why ===" << std::endl;
	setResponseHeaders(server);
	print_request(request);
	response.print_response();
}