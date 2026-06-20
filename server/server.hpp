#pragma once

#include <string>
#include <netinet/in.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <cerrno>
#include <cstdio>


class Server {
public:
    Server(const std::string& host, int port);
    ~Server();

    bool setup();

    int         getFd()   const;
    std::string getHost() const;
    int         getPort() const;

    int acceptConnection(std::string& out_ip, int& out_port) const;

private:
    std::string     _host;
    int             _port;
    int             _fd;
    sockaddr_in     _addr;

    Server(const Server&);
    Server& operator=(const Server&);
};
