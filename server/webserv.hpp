#pragma once

#include <string>
#include <map>
#include <vector>

enum e_method {
    GET    = 0,
    POST   = 1,
    DELETE = 2
};

typedef int err_codes;
typedef int status_code;

enum e_status {
    OK                  = 200,
    CREATED             = 201,
    NO_CONTENT          = 204,
    MOVED_PERMANENTLY   = 301,
    BAD_REQUEST         = 400,
    FORBIDDEN           = 403,
    NOT_FOUND           = 404,
    METHOD_NOT_ALLOWED  = 405,
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED     = 501
};

enum s_state {
    READING  = 0,
    COMPLETE = 1,
    ERROR    = 2
};

struct location {
    std::string                 root;
    std::map<e_method, bool>    methods;
    std::pair<int, std::string> redirection;
    std::string                 index;
    bool                        autoindex;
    std::string                 upload_path;

    location() : autoindex(false) {}
};

typedef std::map<std::string, std::string> strstrMap;
typedef std::map<std::string, location>    strlocationMap;

struct t_server {
    std::string      host;
    int              port;
    std::string      server_name;
    std::string      root;
    size_t           max_body_size;
    unsigned int     upload_counter;
    strlocationMap   locations;
    std::map<int, std::string> err_pages;

    t_server() : port(8080), max_body_size(1048576), upload_counter(0) {}
};
