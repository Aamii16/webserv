#pragma once
#include "utils.h"



typedef enum s_token
{
    SERVER = 1,
    ERR_PAGE,
    LOCATION,
    PORT,
    S_NAME,
    ROOT,
    PATH,
    C_MAX_SIZE,
    IDX,
    AUTO_IDX,
    METHODS,
    GET,
    POST,
    DELETE,
    REDIR,
    CGI,
    PHP,
    JS,
    PY
}   token;

// enum s_status_code
// {

// }   status_code;

typedef struct s_server
{
    std::string                 port;
    std::string                 name;
    std::string                 root;
    std::string                 index;
    long                        max_body_size;
}   t_server;


typedef struct s_location
{
    std::string                         path;
    std::string                         upload_path;
    std::string                         root;
    intboolMap                          methods;
    std::string                         index;
    bool                                auto_idx;
    std::map<int, std::string>          return_directive;
    std::map<std::string, std::string>  cgi_ext;
    bool                                cgi;
}   location;



typedef struct s_configuration
{
    t_server                    server;
    std::map<int, std::string>  err_pages;
    std::vector<location>       locations;
}	t_configuration;

void	parseConf(t_configuration &conf, std::ifstream &file);