#pragma once

#include "Request.hpp"

class RequestParser : public Request
{
	public:
		RequestParser();
		RequestParser(Request &request);
    private:    
        void    parse_token_value(std::string &line, std::string &token, std::string &value, std::string delimiter);
        void    parse_request_line(std::string &line);
        void    parse_header(std::string &header);
        void    parse_body(const size_t &max_body_size);

	public:
		void	parse_request(std::string buffer, const size_t &max_body_size)
};