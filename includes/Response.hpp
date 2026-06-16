#pragma once

#include "Request.hpp"

class Response
{
	private:
		int			status;
		std::string	version;
		std::string	message;
		strstrMap	headers;
		std::string	body;
		std::string response_buffer; 
	public:
		Response();
		Response(int &code);
		~Response();

		//delete this later
		void	print_response()const;
	
		//setters
		void	setStatusCode(err_codes &err);
		void	setStatusCode(status_code &s);
		void	setVersion(std::string &v);
		void	setMessage();
		void	setHeader(std::string key, std::string value);
		void	setBody(std::string &b);
};
