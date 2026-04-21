#pragma once
#include "utils.h"

typedef enum s_state{
	HEADER = 1,
	BODY,
	COMPLETE
}	state;


class Request
{
    int         method;
    int         state;
    std::string target;
    std::string version;
    strstrMap   headers;
    std::string body;
    public:
        std::string r_buffer;
        Request();

        //helper functions
        void	parse_token_value(std::string &line, std::string &token, std::string &value, std::string delimiter);
        void	parse_request_line(std::string &line);
        void	parse_header(std::string header);

        void    parse_request(std::string buffer);
        //getters
        int    getMehod()const;
        std::string    getTarget()const;
        std::string    getVersion()const;
        std::string    getBody()const;
        strstrMap      getHeaders()const;
        std::string    getHeaderValue(std::string &key)const;
        

};