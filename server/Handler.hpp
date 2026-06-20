
//adapted declarations for testing

#pragma once

#include "webserv.hpp"
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <cstring>

class Request;
class Response;

class Handler
{
public:
    Handler(int fd);
    ~Handler();

    void        process(t_server& server, const std::string& buffer);
    s_state     getState() const;
    std::string getResponseString() const;
    void        reset();

private:
    int      _fd;
    s_state  _state;

    Request*  _request;
    Response* _response;

    void handle_request(t_server& server);
    void handle_post(const location& loc, unsigned int& upload_counter);
    void handle_get(const location& loc, std::string& path);
    void handle_delete(std::string& path);
    void setResponseHeaders(const t_server& server);
};
