#include "Connection.hpp"

//
void print_request(const Request &req)
{
    std::cout << "=== HTTP REQUEST ===" << std::endl;
	std::string method_str;
	if (req.getMehod() == GET)
		method_str = "GET";
	else if (req.getMehod() == POST)
		method_str = "POST";
	else if (req.getMehod() == DELETE)
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

void Connection::process(const t_server &server, std::string buffer)
{
	try{
		request.parse_request(buffer, server.max_body_size);
		request.handle_request(server);
	}
	catch(err_codes &error){
		std::cerr << "Error: " << error << std::endl;
		response.setStatusCode(error);
		switch (error)
		{
			case BAD_REQUEST:
				std::cerr << "BAD_REQUEST";
				break;
			case FORBIDDEN:
				std::cerr << "FORBIDDEN";
				break;
			case NOT_FOUND:
				std::cerr << "NOT_FOUND";
				break;
			case METHOD_NOT_ALLOWED:
				std::cerr << "METHOD_NOT_ALLOWED";
				break;
			case REQUEST_TIMEOUT:
				std::cerr << "REQUEST_TIMEOUT";
				break;
			case URI_TOO_LONG:
				std::cerr << "URI_TOO_LONG";
				break;
			case LENGTH_REQUIRED:
				std::cerr << "LENGTH_REQUIRED";
				break;
			case PAYLOAD_TOO_LARGE:
				std::cerr << "PAYLOAD_TOO_LARGE";
				break;
			case HEADER_FIELD_TOO_LARGE:
				std::cerr << "HEADER_FIELD_TOO_LARGE";
				break;
			case INTERNAL_SERVER_ERROR:
				std::cerr << "INTERNAL_SERVER_ERROR";
				break;
			case NOT_IMPLEMENTED:
				std::cerr << "NOT_IMPLEMENTED";
				break;
			case UNSUPPORTED_HTTP:
				std::cerr << "UNSUPPORTED_HTTP";
				break;
			default:
				std::cerr << error;
			std::cerr << std::endl;
		}
		// intstrMap::const_iterator it = server.err_pages.find(error);
		// if (it != server.err_pages.end())
		// 	response.setbody(it->second);
		// else
		// 	response.setbody(errpage(error));
	}
	catch(status_code &s){
		response.setStatusCode(s);
		std::cerr << "status code: ";
		switch (s)
		{
			case OK:
				std::cerr << "OK";
				break;
			case CREATED:
				std::cerr << "CREATED";
				break;
			case NO_CONTENT:
				std::cerr << "NO_CONTENT";
				break;
			case MOVED_PERMANENTLY:
				std::cerr << "MOVED_PERMANENTLY";
				break;
			case FOUND:
				std::cerr << "FOUND";
				break;
			case NOT_MODIFIED:
				std::cerr << "NOT_MODIFIED";
				break;
			default:
				std::cerr << s;
		}
		std::cerr << std::endl;
	}
	print_request(request);

}