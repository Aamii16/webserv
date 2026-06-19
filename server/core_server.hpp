#pragma once

#include "server.hpp"
#include "client.hpp"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>
#include <sstream>
#include <map>
#include <vector>
#include <sys/epoll.h>


class CoreServer {
public:
    CoreServer();
    ~CoreServer();

    bool addServer(const std::string& host, int port);
    void run();

private:

    static const int MAX_EVENTS = 64;


    bool epollAdd(int fd, uint32_t events);
    bool epollMod(int fd, uint32_t events);
    bool epollDel(int fd);

    void handleNewConnection(Server* srv);
    void handleClientRead(int fd);
    void handleClientWrite(int fd);
    void closeClient(int fd);

    void processRequest(Client* client); //place holder


    int _epfd;

    std::vector<Server*>    _servers;
    std::map<int, Server*>  _listenMap;
    std::map<int, Client*>  _clients;

};
