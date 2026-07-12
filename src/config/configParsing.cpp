#include "webserv.h"

bool valid_ip(std::string &ip)
{
    int count = 0;
    size_t i = 0;
    if (ip == "localhost")
        return true;
    for (i = 0;i < ip.size();++i)
        if (ip[i] == '.')
            count++;
    if (count != 3)
        return false;
    count = 0;
    for (i = 0;i < ip.size();++i)
    {
        if (!isdigit(static_cast<unsigned char>(ip[i])))
            return false;
        int num = 0;
        for (;i < ip.size() && ip[i] != '.' && num < 300;++i)
        {
            if (!isdigit(static_cast<unsigned char>(ip[i])))
               return false;
            num = num * 10 + (ip[i] - '0');
        }
        if (num > 255 || num < 0)
            return false;
        count++;
    }
    return (count == 4);
}

bool valid_ip_port(t_server &server)
{
    size_t pos = server.listen.find(":");
    if (pos == std::string::npos)
        return false;
    std::string ip = server.listen.substr(0, pos);
    int port = 0;
    if (pos + 1 >= server.listen.size())
        return false;
    std::string tmp = server.listen.substr(pos + 1);
    for (size_t i = 0; i < tmp.size() && port < 50000 ;++i)
    {
        if (!isdigit(static_cast<unsigned char>(tmp[i])))
            return false;
        port = port * 10 + (tmp[i] - '0');
    }
    if (port < 1024 || port > 49151) // restrict to non-privileged, non-ephemeral ports
        return false;
    for (size_t i = 0; i < server.ports.size(); ++i) {
        if (server.ports[i].first == port && server.ports[i].second == ip) {
            return false;
        }
    }
    server.ports.push_back(std::make_pair(port, ip));
    return (valid_ip(ip));
}

std::string parse_server(t_configuration &conf, std::ifstream &file, std::string &line)
{
    t_server      serv;
    std::string   token;
    std::string   value;
    int           idx;
    
    if (line != "server")
        throw ConfigException("Invalid Conf: " + line );
    if (!std::getline(file, line) || line.find("{") == std::string::npos || line.empty())
        throw ConfigException("Invalid Conf");
    serv.max_body_size = 0;
    while(std::getline(file, line) && !line.empty())
    {
        idx = parseTokenValue(line, token, value, SERVER);
        switch (idx)
        {
            case PORT:
                serv.listen = value;
                if (!valid_ip_port(serv))
                    throw ConfigException("Invalid server ip:port: " + serv.listen);
                break;
            case ROOT:
                serv.root = value;break;
            case SERVER_NAME:
                serv.server_name = value;break;
            case C_MAX_SIZE:
                errno = 0;
                serv.max_body_size = std::strtol(value.c_str(), NULL, 10);
                if (errno == ERANGE)
                    throw ConfigException("Invalid body size: " + line);
                break;
            case 0:
                break;
            default:
                throw ConfigException("Invalid Configuration Token at server: " + line);
        }
    }
    // conf.servers[serv.server_name] = serv;
    conf.servers.insert(std::make_pair(serv.listen, serv));
    if (!serv.max_body_size)
        throw ConfigException("Invalid configuration, missing client_max_body_size");
    // update the upload counter for this server in case of a restart to avoid overwriting existing files
    update_counter(conf.upload_counter_file, conf.servers[serv.listen].upload_counter, 'r');
    return serv.listen;
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

    loc.cgi = false;
    loc.auto_idx = false;
    loc.redirection = std::make_pair(0, std::string());

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
            case UPLOAD_PATH:
                loc.upload_path = value;
                 break;
            case 0:
                break;
            default:
                throw ConfigException("Invalid Configuration Token at location: " + line);
        }
    }
    server.locations[loc.alias] = loc;
}

void    print_conf(t_server &server);

void	parseConf(t_configuration &conf, std::ifstream &file)
{
    std::string line;
    size_t      non_tab;
    int         idx;
    conf.upload_counter_file = "upload_counter.txt";

    try{
        std::getline(file, line);
        std::string port = parse_server(conf, file, line);
        while (std::getline(file, line)){
            non_tab = line.find_first_not_of('\t');
            if (non_tab != 1)
                throw ConfigException("Invalid Config format missing indentation at: " + line);
            idx = valid_token(line, 0);
            switch (idx) {
                case ERR_PAGE:
                    parse_err_pages(conf.servers[port].err_pages, file, line);break;
                case LOCATION:
                    parse_location(conf.servers[port], file, line);break;
                case 0:break;
                default:
                    throw ConfigException("Invalid token at:" + line);
            }
            if (line.find("}") != std::string::npos)
                break ;
        }
        if (line.find("}") == std::string::npos || std::getline(file, line))
            throw ConfigException("Invalid config file: " + line);
    }
    catch(std::exception &e){
        file.close();
        std::cerr << "Error: " << e.what() << std::endl;
        exit(1);
    }
    file.close();
    print_conf(conf.servers.begin()->second);
}

void    print_conf(t_server &server)
{
        std::cout << "=== SERVER ===" << std::endl;
        // print the server's listen addresses and ports
        std::cout << "Listen: ";
        for (size_t i = 0; i < server.ports.size(); ++i)
        {
            std::cout << server.ports[i].second << ":" << server.ports[i].first;
            if (i < server.ports.size() - 1)
                std::cout << ", ";
        }
        std::cout << std::endl;
        // std::cout << "Name: " << server.name << std::endl;
        std::cout << "Root: " << server.root << std::endl;
        std::cout << "Server Name: " << server.server_name << std::endl;
        std::cout << "Max Body Size: " << server.max_body_size << std::endl;

        std::cout << "\n=== ERROR PAGES ===" << std::endl;
        for (std::map<int, std::string>::const_iterator it = server.err_pages.begin(); it != server.err_pages.end(); ++it)
            std::cout << it->first << ": " << it->second << std::endl;

        std::cout << "\n=== LOCATIONS ===" << std::endl;
        for (std::map<std::string, location>::const_iterator loc_it = server.locations.begin(); loc_it != server.locations.end(); ++loc_it)
        {
            std::cout << "\nLocation:" << std::endl;
            std::cout << "  Path: " << loc_it->second.alias << std::endl;
            std::cout << "  Upload Path: " << loc_it->second.upload_path << std::endl;
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
            if (!loc_it->second.redirection.second.empty())
            {
                std::cout << "  Return Directives:" << std::endl;
                std::cout << "    " << loc_it->second.redirection.first << " -> " << loc_it->second.redirection.second << std::endl;
            }
            if (loc_it->second.cgi)
            {
                std::cout << "  CGI Extensions:" << std::endl;
                for (std::map<std::string, std::string>::const_iterator cit = loc_it->second.cgi_ext.begin(); cit != loc_it->second.cgi_ext.end(); ++cit)
                    std::cout << "    " << cit->first << " -> " << cit->second << std::endl;
            }
        }
}
