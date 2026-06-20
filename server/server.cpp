#include "server.hpp"



Server::Server(const std::string& host, int port)
    : _host(host), _port(port), _fd(-1)
{
    std::memset(&_addr, 0, sizeof(_addr));
}

Server::~Server()
{
    if (_fd >= 0)
        close(_fd);
}

int         Server::getFd()   const { return _fd; }
std::string Server::getHost() const { return _host; }
int         Server::getPort() const { return _port; }


bool Server::setup()
{
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0) {
        perror("socket");
        return false;
    }

    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEADDR");
        close(_fd); _fd = -1;
        return false;
    }

    if (fcntl(_fd, F_SETFL, O_NONBLOCK) < 0) {
        perror("fcntl O_NONBLOCK");
        close(_fd); _fd = -1;
        return false;
    }

    _addr.sin_family      = AF_INET;
    _addr.sin_port        = htons(static_cast<uint16_t>(_port));
    _addr.sin_addr.s_addr = _host.empty() ? INADDR_ANY : inet_addr(_host.c_str());

    if (bind(_fd, reinterpret_cast<sockaddr*>(&_addr), sizeof(_addr)) < 0) {
        perror("bind");
        close(_fd); _fd = -1;
        return false;
    }

    if (listen(_fd, SOMAXCONN) < 0) {
        perror("listen");
        close(_fd); _fd = -1;
        return false;
    }

    return true;
}

int Server::acceptConnection(std::string& out_ip, int& out_port) const
{
    sockaddr_in peer;
    socklen_t   len = sizeof(peer);
    std::memset(&peer, 0, sizeof(peer));

    int client_fd = accept(_fd, reinterpret_cast<sockaddr*>(&peer), &len);
    if (client_fd < 0)
        return -1;

    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0) {
        perror("fcntl client O_NONBLOCK");
        close(client_fd);
        return -1;
    }

    out_ip   = inet_ntoa(peer.sin_addr);
    out_port = ntohs(peer.sin_port);
    return client_fd;
}
