#pragma once

#include "server.hpp"
#include "client.hpp"
#include <map>
#include <vector>
#include <sys/epoll.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>

class CoreServer {
public:
    CoreServer();
    ~CoreServer();

    bool addServer(const std::string& host, int port, const t_server& cfg);

    void run();

private:
    bool epollAdd(int fd, uint32_t events);
    bool epollMod(int fd, uint32_t events);
    bool epollDel(int fd);

    void handleNewConnection(int listen_fd);
    void handleClientRead(int fd);
    void handleClientWrite(int fd);
    void closeClient(int fd);

    int                      _epfd;
    std::map<int, t_server>  _configs;
    std::vector<Server*>     _servers;
    std::map<int, Client*>   _clients;

    static const int MAX_EVENTS = 64;
};
