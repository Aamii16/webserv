#pragma once

#include "config.h"
#include "httpRequest.hpp"

class httpResponse
{
	private:
		int	status_code;
		std::string	version;
		std::string	message;
		strstrMap	headers;
		std::string	body;
	public:
		httpResponse();
		httpResponse(int code);
		~httpResponse();
};
