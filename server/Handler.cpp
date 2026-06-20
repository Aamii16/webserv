

//remove before merge

#include "Handler.hpp"


struct Request {
    std::string method, uri, version, body;
    std::map<std::string,std::string> headers;
    bool complete;
    bool error;
    Request() : complete(false), error(false) {}
};

struct Response {
    int         status;
    std::string msg;
    std::string body;
    std::map<std::string,std::string> headers;
    Response() : status(200), msg("OK") {}
};

Handler::Handler(int fd)
    : _fd(fd), _state(READING)
    , _request(new Request)
    , _response(new Response)
{}

Handler::~Handler()
{
    delete _request;
    delete _response;
}

void Handler::process(t_server& server, const std::string& buffer)
{
    if (_state != READING) return;
    (void)server;

    _request->body += buffer;

    if (_request->body.find("\r\n\r\n") == std::string::npos)
        return;

    size_t rl = _request->body.find("\r\n");
    std::string line = _request->body.substr(0, rl);
    size_t s1 = line.find(' ');
    size_t s2 = s1 != std::string::npos ? line.find(' ', s1+1) : std::string::npos;
    if (s1 == std::string::npos || s2 == std::string::npos) {
        _state = ERROR;
        _response->status = 400; _response->msg = "Bad Request";
        _response->body   = "<h1>400 Bad Request</h1>";
        return;
    }
    _request->method  = line.substr(0, s1);
    _request->uri     = line.substr(s1+1, s2-s1-1);
    _request->version = line.substr(s2+1);

    //_state = COMPLETE;
}

s_state Handler::getState() const { return _state; }

std::string Handler::getResponseString() const
{
    std::ostringstream oss;
    oss << "HTTP/1.1 " << _response->status << " " << _response->msg << "\r\n";
    for (std::map<std::string,std::string>::const_iterator it = _response->headers.begin();
         it != _response->headers.end(); ++it)
        oss << it->first << ": " << it->second << "\r\n";
    oss << "Content-Length: " << _response->body.size() << "\r\n";
    oss << "\r\n" << _response->body;
    return oss.str();
}

void Handler::reset()
{
    _state = READING;
    delete _request;
     _request  = new Request;
    delete _response;
    _response = new Response;
}

void Handler::handle_request(t_server&) {}
void Handler::handle_post(const location&, unsigned int&) {}
void Handler::handle_get(const location&, std::string&) {}
void Handler::handle_delete(std::string&) {}
void Handler::setResponseHeaders(const t_server&) {}
