#include "webserv.h"

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

int    valid_token(std::string token, bool block)
{
    std::stringstream    s_token(token);
    s_token >> token;
    if (token.empty())
        return 0;
    if (!block && token == "server")
        return SERVER;
    if (!block && token == "error_pages")
        return ERR_PAGE;
    if (!block && token == "location")
        return LOCATION;
    if (block && token == "listen")
        return PORT;
    if (block && token == "server_name")
        return S_NAME;
    if (block && token == "root")
        return ROOT;
    if (block && token == "path")
        return PATH;
    if (block && token == "client_max_body_size")
        return C_MAX_SIZE;
    if (block && token == "index")
        return IDX;
    if (block && token == "autoindex")
        return AUTO_IDX;
    if (block && token == "methods")
        return METHODS;
    if (block && token == "return")
        return REDIR;
    if (block && token == "cgi")
        return CGI;
    return -1;
}

int parseTokenValue(std::string line, std::string &token, std::string &value)
{
    std::stringstream ss(line);
    ss >> token;
    ss >> value;
    return valid_token(token, 1);
}

void parse_server(t_configuration &conf, std::ifstream &file)
{
    std::string   line;
    std::string   token;
    std::string   value;
    int           idx;

    while(std::getline(file, line) && !line.empty())
    {
        idx = parseTokenValue(line, token, value);
        switch (idx)
        {
            case PORT:
                conf.server.port = value;
                break;
            case S_NAME:
                conf.server.name = value;
                break;
            case ROOT:
                conf.server.root = value;
                break;
            case C_MAX_SIZE:
                errno = 0;
                conf.server.max_body_size = std::strtol(value.c_str(), NULL, 10);
                if (errno == ERANGE)
                    throw ConfigException("Invalid body size: " + line);
                break;
            case IDX:
                conf.server.index = value;
                break;
            case 0:
                break;
            default:
                throw ConfigException("Invalid Configuration Token at server: " + line);
                break; // throw exceptions
        }
    }
}

void    parse_err_pages(std::map<int, std::string> &map, std::ifstream &file)
{
    std::string line;
    std::string token;
    std::string value;
    int           key = 0;
    
    while(std::getline(file, line) && !line.empty())
    {
        parseTokenValue(line, token, value);
        errno = 0;
        key = std::strtol(token.c_str(), NULL, 10);
        if (!key || errno == ERANGE)
            throw ConfigException("Invalid error code: " + token);
        map[key] = value;
    }
}

void    parse_redir(location &loc, std::string &line)
{
    std::stringstream ss(line);
    std::string token, val;
    int key;

    ss >> token;
    ss >> token;
    ss >> val;
    key = std::strtol(token.c_str(), NULL, 10);
    if (!key || errno == ERANGE || val.empty())
        throw ConfigException("Invalid status code at: " + line);
    loc.return_directive[key] = val;
}

void    set_methods(intboolMap &map, std::string &meths)
{
    std::stringstream ss(meths);

    map[GET] = false;
    map[POST] = false;
    while(ss >> meths)
    {
        if(meths == "GET")
            map[GET] = true;
        else if (meths == "POST")
            map[POST] = true;
        else
            throw ConfigException("Invalid method");
    }
}


void    set_cgi(strstrMap &map, std::string &cgi)
{
    std::string key;
    std::string value;
    std::stringstream   ss(cgi);
    ss >> cgi;
    ss >> key;
    if (key != ".py" && key != ".php" && key != ".js")
        throw ConfigException("Invalid cgi extension");
    ss >> value;
    map[key] = value;
    if (map[key].empty())
        throw ConfigException("Invalid cgi extension");
}

void    parse_location(t_configuration &conf, std::ifstream &file)
{
    std::string line;
    std::string token;
    std::string value;
    location    loc;
    int         idx;
    
    while(std::getline(file, line) && !line.empty())
    {
        idx = parseTokenValue(line, token, value);
        switch (idx)
        {
            case PATH:
                loc.path = value;
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
                parse_redir(loc, line);
                break;
            case CGI:
                set_cgi(loc.cgi_ext, line);
                loc.cgi = true;
                break;
            case METHODS:
                set_methods(loc.methods, value);
                break;
            case 0:
                break;
            default:
                throw ConfigException("Invalid Configuration Token at location: " + line);
                break; // throw exceptions
        }
    }
    conf.locations.push_back(loc);
}

void parse_block(t_configuration &conf, std::ifstream &file, int idx)
{
    switch (idx)
    {
        case 1:
            parse_server(conf, file);
            break;
        case 2:
            parse_err_pages(conf.err_pages, file);
            break;
        case 3:
            parse_location(conf, file);
            break;
        case 0:
            break;
        default:
            throw ConfigException("Invalid token");
    }   
}

void	parseConf(t_configuration &conf, std::ifstream &file)
{
    //need to throw error if not valid indentation
    std::string line;
    int token;
    
    try
    {
        while (std::getline(file, line))
        {
            token = valid_token(line, 0);
            parse_block(conf, file, token);
        }
    }
    catch(std::exception &e)
    {
        file.close();
        std::cerr << "An exception occured: " << e.what() << std::endl;
        exit(1);
    }
    file.close();
    // {
    //     std::cout << "=== SERVER ===" << std::endl;
    //     std::cout << "Port: " << conf.server.port << std::endl;
    //     std::cout << "Name: " << conf.server.name << std::endl;
    //     std::cout << "Root: " << conf.server.root << std::endl;
    //     std::cout << "Index: " << conf.server.index << std::endl;
    //     std::cout << "Max Body Size: " << conf.server.max_body_size << std::endl;

    //     std::cout << "\n=== ERROR PAGES ===" << std::endl;
    //     for (std::map<int, std::string>::const_iterator it = conf.err_pages.begin(); it != conf.err_pages.end(); ++it)
    //         std::cout << it->first << ": " << it->second << std::endl;

    //     std::cout << "\n=== LOCATIONS ===" << std::endl;
    //     for (size_t i = 0; i < conf.locations.size(); ++i)
    //     {
    //         std::cout << "\nLocation " << i << ":" << std::endl;
    //         std::cout << "  Path: " << conf.locations[i].path << std::endl;
    //         std::cout << "  Root: " << conf.locations[i].root << std::endl;
    //         std::cout << "  Index: " << conf.locations[i].index << std::endl;
    //         std::cout << "  Autoindex: " << conf.locations[i].auto_idx << std::endl;
    //         std::cout << "  CGI: " << conf.locations[i].cgi << std::endl;
    //         std::cout << "  Methods:\n  GET: " << conf.locations[i].methods[GET] << "\tPOST: " << conf.locations[i].methods[GET] << std::endl;
    //     }
    // }
}