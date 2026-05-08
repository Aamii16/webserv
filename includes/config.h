#pragma once
#include "utils.h"


typedef std::map<std::string, std::string> strstrMap;
typedef std::map<int, bool> intboolMap;
typedef std::map<int, std::string> intstrMap;
typedef std::map<std::string, int> strintMap;


typedef enum s_token
{
	ERR_PAGE = 1,
	SERVER,
	LOCATION,
	PORT,
	S_NAME,
	ROOT,
	PATH,
	C_MAX_SIZE,
	IDX,
	AUTO_IDX,
	METHODS,
	GET,
	POST,
	DELETE,
	REDIR,
	CGI,
	PHP,
	JS,
	PY
}   token;

// enum s_status_code
// {

// }   status_code;

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
	std::map<int, std::string>          return_directive;
	std::map<std::string, std::string>  cgi_ext;
	bool                                cgi;
}   location;

typedef std::map<std::string, location> strlocationMap;

typedef struct s_server
{
	std::string						port;
	std::string						name;
	std::string						root;
	std::string						index;
	long       						max_body_size;
	std::map<int, std::string>		err_pages;	
	std::map<std::string, location>	locations;
}   t_server;	



typedef struct s_configuration
{
	std::map<std::string, t_server>	servers; // key == port number

}	t_configuration;


//utils
int		valid_token(std::string line, int block);
int		parseTokenValue(std::string line, std::string &token, std::string &value, int block);
void	parse_redir(location &loc, std::string &line);
void	set_methods(intboolMap &map, std::string &meths);
void	set_cgi(strstrMap &map, std::string &cgi);

void	parseConf(t_configuration &conf, std::ifstream &file);
