#pragma once

#include "utils.h"
#include "config.h"
#include "webserv.h"

typedef enum s_state{
    REQ_LINE = 1,
	HEADERS,
	BODY,
	COMPLETE,
    BAD_REQUEST
}	state;


class Request
{
    int	err_code;
    int         method;
    int         state;
    size_t      content_length;
    std::string target;
    std::string version;
    std::string query;
    strstrMap   headers;
    std::string body;
    public:
        std::string r_buffer;
        Request();

    //helper functions
    private:    
        void	parse_token_value(std::string &line, std::string &token, std::string &value, std::string delimiter);
        void	parse_request_line(std::string &line);
        void	parse_header(std::string header);
        void    parse_body();
    public:
        void    parse_request(std::string buffer);
        int     process(t_configuration &conf);
        //getters
        int            getMehod()const;
        int            getState()const;
        size_t         getContentLength();
        std::string    getTarget()const;
        std::string    getVersion()const;
        std::string    getQuery()const;
        std::string    getBody()const;
        strstrMap      getHeaders()const;
        std::string    getHeaderValue(std::string &key)const;
        

};