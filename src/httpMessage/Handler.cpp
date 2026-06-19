#include "Handler.hpp"


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

Handler::Handler(int fd): fd(fd){}


void Handler::process(t_server &server, std::string buffer)
{
	if (buffer.empty() && request.getState() != COMPLETE)
		return ;
	try{
		request.parse_request(buffer, server.max_body_size);
		if (request.getState() == COMPLETE)
			handle_request(server);
		else
			return ;
	}
	catch(err_codes &error){
		request.setState(ERROR);
		state = ERROR; // i'll have to find a better way to handle this state stuff later
		response.setStatusCode(error);
		response.setHeader("Content-Type", "text/html");
		std::cout << "Error Code: " << error << " msg: " << response.getMessage() << std::endl;
		// intstrMap::const_iterator it = server.err_pages.find(error);
		// if (it != server.err_pages.end())
		// 	response.setbody(it->second);
		// else
		// 	response.setbody(errpage(error));
	}
	catch(status_code &status){
		request.setState(COMPLETE);
		state = COMPLETE;
		response.setStatusCode(status);
		std::cout << "status code: " << response.getMessage() << std::endl;
	}
	if (state == ERROR || state == COMPLETE)
	{
		setResponseHeaders(server);
		print_request(request);
		response.print_response();
	}
}

void Handler::reset()
{
	request = Request();
	response = Response();
	state = REQ_LINE;
}