#pragma once

#include "Response.hpp"

class Handler
{
	int fd;
	// bool is_open;
	// bool readeable;
	// bool writeable;
	Request		request;
	Response	response;
	public:
		s_state state;
		Handler(int fd);


		void    handle_request(t_server &server);
		void	process(t_server &server, std::string buffer);
		void	handle_cgi(const location &loc, std::string &path, const t_server &server);
		void	handle_post(const location &loc, unsigned int &upload_counter);
		void	handle_get(const location &loc, std::string &path);
		void	handle_delete(std::string &path);
		void	setResponseHeaders(const t_server &server);

		s_state       getState() const;
		std::string getResponseString();
		void        reset(); 

};

const std::string get_extension(const std::string &content_type);
