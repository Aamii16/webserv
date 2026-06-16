#pragma once

#include "utils.h"

class Response;

typedef enum s_state{
    REQ_LINE = 1,
	HEADERS,
	BODY,
	COMPLETE,
    ERROR
}	state;

typedef	enum err_codes
{
	BAD_REQUEST = 400,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	REQUEST_TIMEOUT = 408,
	LENGTH_REQUIRED = 411,
	PAYLOAD_TOO_LARGE = 413,
	URI_TOO_LONG = 414,
	HEADER_FIELD_TOO_LARGE = 431,
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501,
    GATEWAY_TIMEOUT = 504,
	UNSUPPORTED_HTTP = 505,
    INSUFFICIENT_STORAGE = 507
}	err_codes;

typedef enum status_code
{
	OK = 200,
	CREATED = 201,
	NO_CONTENT = 204,
	MOVED_PERMANENTLY = 301,
	FOUND = 302,
	NOT_MODIFIED = 304,
}	status_code;

class Request
{
    int         err_code;
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
        void    parse_token_value(std::string &line, std::string &token, std::string &value, std::string delimiter);
        void    parse_request_line(std::string &line);
        void    parse_header(std::string &header);
        void    parse_body(const size_t &max_body_size);
    public:
        void    parse_request(std::string buffer, const size_t &max_body_size);
        //getters
        int         getMethod()const;
        int         getState()const;
        size_t      getContentLength();
        std::string getTarget()const;
        std::string getVersion()const;
        std::string getQuery()const;
        std::string getBody()const;
        strstrMap   getHeaders()const;
        std::string getHeaderValue(const std::string &key)const;

        void        setTarget(std::string &t){target = t;}
        void        setContentLength();
};