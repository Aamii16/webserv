#pragma once

#include "server.hpp"
#include "client.hpp"
#include "CGIHandler.hpp"
#include <map>
#include <vector>
#include <ctime>
#include <sys/epoll.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>

struct CgiState {
    CGIHandler  *cgi;
    int          client_fd;
    time_t       start_time;

    CgiState(CGIHandler *c, int fd) : cgi(c), client_fd(fd), start_time(time(NULL)) {}
};

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

    // CGI / epoll integration
    void registerCgi(int client_fd, CGIHandler *cgi);
    void handleCgiEvent(int fd, uint32_t events);
    void finishCgi(CgiState *st);
    void checkCgiTimeouts();
    void dropCgiState(CgiState *st, bool killChild);

    int                      _epfd;
    std::map<int, t_server>  _configs;
    std::vector<Server*>     _servers;
    std::map<int, Client*>   _clients;

    std::map<int, CgiState*> _cgiByFd;      // stdin_fd and/or stdout_fd -> state
    std::map<int, CgiState*> _cgiByClient;  // client_fd -> state

    static const int MAX_EVENTS = 64;
    static const int CGI_TIMEOUT_SECONDS = 10;
};
