#include "Handler.hpp"

std::string errpage(int code)
{
	std::string error_body = 
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    "    <meta charset=\"utf-8\">\n"
    "    <title>Error " + num_to_str(code) + ": " + getCodeMessage(code) + "</title>\n"
    "    <style>\n"
    "        body { font-family: -apple-system, sans-serif; text-align: center; padding: 5% 20px; background: #fafafa; color: #333; }\n"
    "        .card { max-width: 500px; margin: 0 auto; background: #fff; padding: 40px; border-radius: 8px; box-shadow: 0 4px 15px rgba(0,0,0,0.05); }\n"
    "        h1 { font-size: 72px; margin: 0; color: #e04444; font-weight: 300; }\n"
    "        p { font-size: 18px; color: #666; margin: 20px 0 30px; }\n"
    "        a { text-decoration: none; color: #0066cc; font-size: 14px; }\n"
    "        a:hover { text-decoration: underline; }\n"
    "    </style>\n"
    "</head>\n"
    "<body>\n"
    "    <div class=\"card\">\n"
    "        <h1>" + num_to_str(code) + "</h1>\n"
    "        <p>" + getCodeMessage(code) + "</p>\n"
    "        <a href=\"/\">&larr; Back to Home</a>\n"
    "    </div>\n"
    "</body>\n"
    "</html>";
	return error_body;
}

std::string Response::geterrpage(const t_server &server, int code)
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
		else
		{
			if (errno == ENOENT)
				std::cerr << "Error: Custom error page not found: " << it->second << std::endl;
			else if (errno == EACCES)
				std::cerr << "Error: Permission denied for custom error page: " << it->second << std::endl;
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


void Handler::setResponseHeaders(const t_server &server)
{
	std::string date;

	response.setVersion(request.getVersion());
	response.setMessage();
	
	// if (!response.getBody().empty() && !response.getHeader("Content-Type").empty())
	// 	response.setHeader("Content-Type", request.getHeaderValue("Content-Type"));
	response.setHeader("Content-Length", num_to_str(response.getBody().size()));
	response.setHeader("Server", server.server_name);
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
