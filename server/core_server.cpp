#include "core_server.hpp"



CoreServer::CoreServer() : _epfd(-1)
{
    _epfd = epoll_create1(EPOLL_CLOEXEC);
    if (_epfd < 0)
        perror("epoll_create1");
}

CoreServer::~CoreServer()
{
    for (std::map<int, Client*>::iterator i = _clients.begin();
         i != _clients.end(); ++i)
        delete i->second;

    for (size_t y = 0; y < _servers.size(); ++y)
        delete _servers[y];

    if (_epfd >= 0)
        close(_epfd);
}

void CoreServer::handleNewConnection(Server* srv)
{
    std::string peer_ip;
    int         peer_port = 0;

    int client_fd = srv->acceptConnection(peer_ip, peer_port);
    if (client_fd < 0)
        return;

    if (!epollAdd(client_fd, EPOLLIN)) {
        close(client_fd);
        return;
    }

    Client* client = new Client(client_fd, peer_ip, peer_port);
    _clients[client_fd] = client;

    printf("New client fd=%d from %s:%d\n", client_fd, peer_ip.c_str(), peer_port);
}

void CoreServer::handleClientRead(int fd)
{
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it == _clients.end())
        return;

    Client* client = it->second;

    ssize_t n = client->recv();

    if (n == 0) {
        closeClient(fd);
        return;
    }
    if (n < 0) { // nsit possible errors should write them
        closeClient(fd);
        return;
    }

    // processRequest(client);

    if (client->wantWrite()) {
        epollMod(fd, EPOLLIN | EPOLLOUT);
        client->setState(STATE_WRITING);
    }
}


//advance on buffer 
void CoreServer::handleClientWrite(int fd)
{
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it == _clients.end())
        return;

    Client* client = it->second;

    ssize_t n = client->send();

    if (n < 0) {
        closeClient(fd);
        return;
    }

    if (!client->wantWrite()) {
        // figure out buffer looping thing
        epollMod(fd, EPOLLIN);
        client->setState(STATE_READING);
    }
}

void CoreServer::closeClient(int fd) //delete epoll_fd first
{
    // std::map<int, Client*>::iterator  = _clients.find(fd);
    // if ( == _clients.end())
    //     return;
}

bool CoreServer::epollAdd(int fd, uint32_t events)
{
    epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.events   = events;
    ev.data.fd  = fd;

    if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        perror("epoll_ctl ADD");
        return false;
    }
    return true;
}

bool CoreServer::epollMod(int fd, uint32_t events)
{
    epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.events   = events;
    ev.data.fd  = fd;

     // do it manually mb
    if (epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev) < 0) { 
        perror("epoll_ctl mod failef");
        return false;
    }
    return true;
}

bool CoreServer::epollDel(int fd)
{
    epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));

    if (epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, &ev) < 0) {
        perror("epoll_ctl DEL failed");
        return false;
    }
    return true;
}

bool CoreServer::addServer(const std::string& host, int port)
{
    Server* srv = new Server(host, port);
    if (!srv->setup()) {
        delete srv;
        return false;
    }

    if (!epollAdd(srv->getFd(), EPOLLIN)) {
        delete srv;
        return false;
    }

    _servers.push_back(srv);
    _listenMap[srv->getFd()] = srv;

    printf("Listening on %s:%d (fd=%d)\n", host.c_str(), port, srv->getFd());
    return true;
}

void CoreServer::run()
{
    if (_epfd < 0) {
        fprintf(stderr, "epoll not initialised\n");
        return;
    }

    epoll_event events[MAX_EVENTS];

    printf("Loop started\n");

    while (true)
    {
        int nready = epoll_wait(_epfd, events, MAX_EVENTS, -1);
        if (nready < 0) {
            perror("epoll_wait failed");
            break;          // ai said could be handled differntly
        }

        for (int i = 0; i < nready; ++i) 
        {
            int fd = events[i].data.fd;

            if (_listenMap.count(fd)) {
                handleNewConnection(_listenMap[fd]);
                continue;
            }

            if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                closeClient(fd);
                continue;
            }

            if (events[i].events & EPOLLIN)
                handleClientRead(fd);

            if (events[i].events & EPOLLOUT)
                handleClientWrite(fd);
        }
    }
}