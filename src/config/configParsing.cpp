#include "webserv.h"

std::string parse_server(t_configuration &conf, std::ifstream &file, std::string &line)
{
    t_server      serv;
    std::string   token;
    std::string   value;
    int           idx;
    
    if (line != "server")
        throw ConfigException("Invalid Conf: " + line );
    if (!std::getline(file, line) || line != "{")
        throw ConfigException("Invalid Conf");
    while(std::getline(file, line) && !line.empty())
    {
        idx = parseTokenValue(line, token, value, SERVER);
        switch (idx)
        {
            case PORT:
                serv.port = value;break;
            case S_NAME:
                serv.name = value;break;
            case ROOT:
                serv.root = value;break;
            case C_MAX_SIZE:
                errno = 0;
                serv.max_body_size = std::strtol(value.c_str(), NULL, 10);
                if (errno == ERANGE)
                    throw ConfigException("Invalid body size: " + line);
                break;
            case IDX:
                serv.index = value;break;
            case 0:
                break;
            default:
                throw ConfigException("Invalid Configuration Token at server: " + line);
        }
    }
    conf.servers[serv.port] = serv;
    return serv.port;
}

void    parse_err_pages(std::map<int, std::string> &map, std::ifstream &file, std::string &line)
{
    std::string token;
    std::string value;
    int           key = 0;
    
    while(std::getline(file, line) && !line.empty() and line.find("}") == std::string::npos)
    {
        parseTokenValue(line, token, value, 1);
        errno = 0;
        key = std::strtol(token.c_str(), NULL, 10);
        if (!key || errno == ERANGE)
            throw ConfigException("Invalid error code: " + token);
        map[key] = value;
    }
}


void    parse_location(t_server &server, std::ifstream &file, std::string &line)
{
    std::string token;
    std::string value;
    location    loc;
    
    while(std::getline(file, line) && !line.empty() && line.find("}") == std::string::npos)
    {
        int idx = parseTokenValue(line, token, value, 1);
        switch (idx)
        {
            case PATH:
                loc.alias = value;
                break;
            case ROOT:
                loc.root = value;
                break;
            case IDX:
                loc.index = value;
                break;
            case AUTO_IDX:
                loc.auto_idx = (value == "true");
                break;
            case REDIR:
                parse_redir(loc, value);
                break;
            case CGI:
                set_cgi(loc.cgi_ext, value);
                loc.cgi = true;
                break;
            case METHODS:
                set_methods(loc.methods, value);
                break;
            case 0:
                break;
            default:
                throw ConfigException("Invalid Configuration Token at location: " + line);
        }
    }
    server.locations[loc.alias] = loc;
}


void    print_conf(t_configuration &conf);

void	parseConf(t_configuration &conf, std::ifstream &file)
{
    std::string line;
    size_t      non_tab;
    int         idx;

    try{
        while (std::getline(file, line)){
            std::string port = parse_server(conf, file, line);
            while (std::getline(file, line)){
                non_tab = line.find_first_not_of('\t');
                if (non_tab != 1) //accept or reject empty lines ? 
                    throw ConfigException("Invalid Config format wrong indentation at: " + line);
                idx = valid_token(line, 0);
                if (idx == ERR_PAGE)
                    parse_err_pages(conf.servers[port].err_pages, file, line);
                else if (idx == LOCATION)
                    parse_location(conf.servers[port], file, line);
                else if (idx == 0)
                    continue ;
                else
                    throw ConfigException("Invalid token at:" + line);
                if (line.find("}") != std::string::npos)
                    break ;
            }
            if (line != "}")
                throw ConfigException("Invalid config missing '}' at:");
        }
    }
    catch(std::exception &e){
        file.close();
        std::cerr << "An exception occured: " << e.what() << std::endl;
        exit(1);
    }
    file.close();
    print_conf(conf);
}

void    print_conf(t_configuration &conf)
{
    for (std::map<std::string, t_server>::const_iterator server_it = conf.servers.begin(); server_it != conf.servers.end(); ++server_it)
    {
        std::cout << "=== SERVER ===" << std::endl;
        std::cout << "Port: " << server_it->second.port << std::endl;
        std::cout << "Name: " << server_it->second.name << std::endl;
        std::cout << "Root: " << server_it->second.root << std::endl;
        std::cout << "Index: " << server_it->second.index << std::endl;
        std::cout << "Max Body Size: " << server_it->second.max_body_size << std::endl;

        std::cout << "\n=== ERROR PAGES ===" << std::endl;
        for (std::map<int, std::string>::const_iterator it = server_it->second.err_pages.begin(); it != server_it->second.err_pages.end(); ++it)
            std::cout << it->first << ": " << it->second << std::endl;

        std::cout << "\n=== LOCATIONS ===" << std::endl;
        for (std::map<std::string, location>::const_iterator loc_it = server_it->second.locations.begin(); loc_it != server_it->second.locations.end(); ++loc_it)
        {
            std::cout << "\nLocation:" << std::endl;
            std::cout << "  Path: " << loc_it->second.alias << std::endl;
            std::cout << "  Root: " << loc_it->second.root << std::endl;
            std::cout << "  Index: " << loc_it->second.index << std::endl;
            std::cout << "  Autoindex: " << loc_it->second.auto_idx << std::endl;
            std::cout << "  CGI: " << loc_it->second.cgi << std::endl;
            std::cout << "  Methods:" << std::endl;
            std::map<int, bool>::const_iterator mit;
            mit = loc_it->second.methods.find(GET);
            if (mit != loc_it->second.methods.end() && mit->second)
                std::cout << "    GET" << std::endl;
            mit = loc_it->second.methods.find(POST);
            if (mit != loc_it->second.methods.end() && mit->second)
                std::cout << "    POST" << std::endl;
            mit = loc_it->second.methods.find(DELETE);
            if (mit != loc_it->second.methods.end() && mit->second)
                std::cout << "    DELETE" << std::endl;
            if (!loc_it->second.return_directive.empty())
            {
                std::cout << "  Return Directives:" << std::endl;
                for (std::map<int, std::string>::const_iterator rit = loc_it->second.return_directive.begin(); rit != loc_it->second.return_directive.end(); ++rit)
                    std::cout << "    " << rit->first << " -> " << rit->second << std::endl;
            }
            if (loc_it->second.cgi)
            {
                std::cout << "  CGI Extensions:" << std::endl;
                for (std::map<std::string, std::string>::const_iterator cit = loc_it->second.cgi_ext.begin(); cit != loc_it->second.cgi_ext.end(); ++cit)
                    std::cout << "    " << cit->first << " -> " << cit->second << std::endl;
            }
        }
    }
}