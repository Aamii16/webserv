#pragma once
#include "Request.hpp"
#include "Response.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>


class CGIHandler
{
	std::vector<std::string> vars;
	char						**env;
	std::string					script;      
	std::string					target_path;
	std::string					output;      
	std::string					body;        
	size_t						body_offset; 
	pid_t						pid
	int							stdin_fd;    
	int							stdout_fd;   

	public:
		CGIHandler(std::string _script);
		~CGIHandler();

		void setEnvVars(const Request& request, const std::string& script_path, const std::string &script_name , const t_server& server);

		bool start();               
		bool hasBodyToWrite() const;
		int  writeStdin();
		int  readStdout();


		void closeStdin();
		void closeStdout();

		void reap(int &exit_status);
		int   getStdinFd()  const;
		int   getStdoutFd() const;
		pid_t getPid()      const;
		std::string getOutput() const;

		void        parseHeaders(Response &response); 
		std::string parseBody() const;                

};
