#include "Handler.hpp"
#include "CGIHandler.hpp"


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

Handler::Handler(int fd) : fd(fd), cgi(NULL), state(READING) {}

Handler::~Handler()
{
	delete cgi;
}

void Handler::process(t_server &server, std::string buffer)
{
	if (state == CGI_WAIT || (buffer.empty() && request.getState() != COMPLETE))
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
		response.setBody(response.geterrpage(server, error));
	}
	catch(status_code &status){
		request.setState(COMPLETE);
		state = COMPLETE;
		response.setStatusCode(status);
	}
	if (state == CGI_WAIT)
		return ; // handle_cgi started with mo issue
	if (state == ERROR || state == COMPLETE)
	{
		setResponseHeaders(server);
		// print_request(request);
		response.print_response();
	}
}

state Handler::getState() const
{
    return state;  
}

std::string Handler::getResponseString()
{
    return response.mkResponse();
}

void Handler::reset()
{
	delete cgi;
	cgi = NULL;
    request  = Request();
    response = Response();
    state    = REQ_LINE;
}

bool Handler::isCgiPending() const
{
	return state == CGI_WAIT;
}

CGIHandler* Handler::getCgi() const
{
	return cgi;
}

void Handler::finishCgi(const t_server &server)
{
	if (!cgi)
		return ;
	cgi->parseHeaders(response);
	response.setBody(cgi->parseBody());
	delete cgi;
	cgi = NULL;
	request.setState(COMPLETE);
	state = COMPLETE;
	setResponseHeaders(server);
	response.print_response();
}

void Handler::abortCgi(const t_server &server, int errcode)
{
	delete cgi;
	cgi = NULL;
	err_codes ec = static_cast<err_codes>(errcode);
	request.setState(ERROR);
	state = ERROR;
	response.setStatusCode(ec);
	response.setHeader("Content-Type", "text/html");
	response.setBody(response.geterrpage(server, errcode));
	setResponseHeaders(server);
	response.print_response();
}