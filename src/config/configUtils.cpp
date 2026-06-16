#include "config.h"

int    valid_token(std::string line, int block)
{
    std::string         token;
    std::stringstream    s_token(line);

    s_token >> token;
    if (token.empty())
        return 0;
    if (!block)
    {
        if (token == "error_pages")
            return ERR_PAGE;
        if (token == "location")
            return LOCATION;
    }
    if (block == SERVER)
    {
        if (token == "listen")
            return PORT;
        else if (token == "root")
            return ROOT;
        else if (token == "client_max_body_size")
            return C_MAX_SIZE;
        else
            return -1;
    }
    if (token == "root")
        return ROOT;
    else if (token == "path")
        return PATH;
    else if (token == "upload_path")
        return UPLOAD_PATH;
    else if (token == "index")
        return IDX;
    else if (token == "autoindex")
        return AUTO_IDX;
    else if (token == "methods")
        return METHODS;
    else if (token == "return")
        return REDIR;
    else if (token == "cgi")
        return CGI;
    return -1;
}

int parseTokenValue(std::string line, std::string &token, std::string &value, int block)
{
    std::stringstream ss(line);
    size_t  non_tab = line.find_first_not_of('\t');

    if (line.find("}") != std::string::npos)
        return 0;
    ss >> token;
    if (line.size() > non_tab + token.size() + 1)
        value = line.substr(non_tab + token.size() + 1);
    else
        throw ConfigException("Invalid Config at: ");
    if (!token.empty())
    {
        if (block == SERVER && non_tab != 1)
            throw ConfigException("Invalid Config format at : " + token);
        else if (block != SERVER && non_tab != 2)
            throw ConfigException("Invalid Config format at : " + token);
    }
    else if (value.empty())
        throw ConfigException("Invalid Config");
    size_t idx = value.find_last_not_of(" \t");
    if (idx != std::string::npos)
        value = value.substr(0, idx + 1);
    return valid_token(token, block);
}

void    parse_redir(location &loc, std::string &line)
{
    std::stringstream ss(line);
    std::string token, val;
    int key;

    ss >> token;
    ss >> val;
    for (size_t i = 0; i < token.size(); ++i)
    {
        if (!std::isdigit(static_cast<unsigned char>(token[i])))
            throw ConfigException("Invalid status_code code at: " + line);
    }
    key = std::strtol(token.c_str(), NULL, 10);
    if (!key || errno == ERANGE || val.empty())
        throw ConfigException("Invalid status_code code at: " + line);
    loc.return_directive[key] = val;
}

void    set_methods(intboolMap &map, std::string &meths)
{
    std::stringstream ss(meths);
    map[GET] = false;
    map[POST] = false;
    map[DELETE] = false;
    while(ss >> meths)
    {
        if(meths == "GET")
            map[GET] = true;
        else if (meths == "POST")
            map[POST] = true;
        else if (meths == "DELETE")
            map[DELETE] = true;
        else
            throw ConfigException("Invalid method");
    }
}


void    set_cgi(strstrMap &map, std::string &cgi)
{
    std::string key;
    std::string value;
    std::stringstream   ss(cgi);
    // ss >> cgi; // why did i comment this out? it was to skip the "cgi" token but i already do that in parseTokenValue so it was redundant
    ss >> key;
    if (key != ".py" && key != ".php" && key != ".js")
        throw ConfigException("Invalid cgi extension");
    ss >> value;
    map[key] = value;
    if (map[key].empty())
        throw ConfigException("Invalid cgi extension");
}

void update_counter(const std::string &name, unsigned int &counter, int flag)
{
	std::fstream file;

	if (flag == 'r')
	{
		file.open(name.c_str(), std::ios::in);
		if (file.is_open())
		{
			file >> counter;
            file.close();
		}
		else
			counter = 0;
	}
	else if (flag == 'w')
	{
		file.open(name.c_str(), std::ios::out | std::ios::trunc);
		if (file.is_open())
		{
			file << counter;
			file.close();
		}
        else
            std::cerr << "Unable to open counter file for writing: " << name << std::endl;
	}
}