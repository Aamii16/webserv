#pragma once

#include "Response.hpp"

class Connection
{
	int fd;
	// bool is_open;
	// bool readeable;
	// bool writeable;
	Request		request;
	Response	response;
	public:
		Connection(int fd);


		void	handle_post(const location &loc, unsigned int &upload_counter);
		void    handle_request(t_server &server);
		void	process(t_server &server, std::string buffer);
		void	setResponseHeaders(const t_server &server);

};