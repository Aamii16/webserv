#pragma once
#include "utils.h"


typedef std::map<std::string, std::string> strstrMap;
typedef std::map<int, bool> intboolMap;
typedef std::map<int, std::string> intstrMap;
typedef std::map<std::string, int> strintMap;
typedef std::pair<int, std::string> intstrPair;


typedef enum s_token
{
	ERR_PAGE = 1,
	SERVER,
	SERVER_NAME,
	LOCATION,
	PORT,
	ROOT,
	PATH,
	UPLOAD_PATH,
	C_MAX_SIZE,
	IDX,
	AUTO_IDX,
	METHODS,
	GET,
	POST,
	DELETE,
	UNKNOWN_METHOD,
	REDIR,
	CGI,
	PHP,
	JS,
	PY
}   token;

// enum s_status_code_code
// {

// }   status_code_code;

class ConfigException : public std::exception
{
    private:
        std::string err;
    public:
        ConfigException(const std::string _err)
        {
            err = _err;
        }
        virtual ~ConfigException() throw()
        {}
        virtual const char* what() const throw()
        {
            return err.c_str();
        }
};

typedef struct s_location
{
	std::string                         alias;
	std::string                         upload_path;
	std::string                         root;
	std::map<int, bool>                 methods;
	std::string                         index;
	bool                                auto_idx;
	std::pair<int, std::string>          redirection;
	std::map<std::string, std::string>  cgi_ext;
	bool                                cgi;
}   location;

typedef std::map<std::string, location> strlocationMap;

struct t_server
{
	std::string      				host;
    std::string				      	server_name;
	std::string						listen;
	std::string						server_name;
	std::string						ip;
	int								port;
	std::string						root;
	long       						max_body_size;
	intstrMap						err_pages;	
	std::map<std::string, location>	locations;
	// upload counter to generate unique file names for uploads, stored in a file to persist across server restarts
	unsigned int 					upload_counter;
	

	t_server() : port(8080), max_body_size(1048576), upload_counter(0) {}
}	;	



typedef struct s_configuration
{
	std::map<std::string, t_server>	servers; // key == port number
	std::string						upload_counter_file;
}	t_configuration;


// parsing utils
int		valid_token(std::string line, int block);
int		parseTokenValue(std::string line, std::string &token, std::string &value, int block);
void	parse_redir(location &loc, std::string &line);
void	set_methods(intboolMap &map, std::string &meths);
void	set_cgi(strstrMap &map, std::string &cgi);

// save and update the upload counter to generate unique file names for uploads, stored in a file to persist across server restarts
void	update_counter(const std::string &name, unsigned int &counter, int flag);

void	parseConf(t_configuration &conf, std::ifstream &file);

