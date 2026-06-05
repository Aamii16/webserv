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
		void process(const t_server &server, std::string buffer);
};