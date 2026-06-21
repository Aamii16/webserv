#include "core_server.hpp"



CoreServer::CoreServer() : _epfd(-1)
{
// EPOLL_CLOEXEC so epoll fd don't leak into CGI child processes
    _epfd = epoll_create1(EPOLL_CLOEXEC); 
    if (_epfd < 0)
        perror("epoll_create1");
}

CoreServer::~CoreServer()
{
    for (std::map<int, Client*>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
        delete it->second;
    for (size_t i = 0; i < _servers.size(); ++i)
        delete _servers[i];
    if (_epfd >= 0)
        close(_epfd);
}

bool CoreServer::addServer(const std::string& host, int port, const t_server& cfg)
{
    Server* srv = new Server(host, port);
    if (!srv->setup())               { delete srv; return false; }
    if (!epollAdd(srv->getFd(), EPOLLIN)) { delete srv; return false; }

    _configs[srv->getFd()] = cfg;
    _servers.push_back(srv);

    printf("[CoreServer] Listening on %s:%d (fd=%d)\n",
           host.c_str(), port, srv->getFd());
    return true;
}

void CoreServer::run()
{
    if (_epfd < 0) { fprintf(stderr, "epoll not initialised\n"); return; }

    epoll_event events[MAX_EVENTS];
    printf("[CoreServer] Event loop started\n");

    while (true)
    {
        int nready = epoll_wait(_epfd, events, MAX_EVENTS, -1);
        if (nready < 0) { perror("epoll_wait"); break; }

        for (int i = 0; i < nready; ++i)
        {
            int fd = events[i].data.fd;

            if (_configs.count(fd)) {
                handleNewConnection(fd);
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

void CoreServer::handleNewConnection(int listen_fd)
{
    Server* srv = NULL;
    for (size_t i = 0; i < _servers.size(); ++i)
        if (_servers[i]->getFd() == listen_fd) { srv = _servers[i]; break; }
    if (!srv) return;

    std::string peer_ip;
    int         peer_port = 0;
    int client_fd = srv->acceptConnection(peer_ip, peer_port);
    if (client_fd < 0) return;

    if (!epollAdd(client_fd, EPOLLIN)) { close(client_fd); return; }

    _clients[client_fd] = new Client(client_fd, peer_ip, peer_port,
                                     _configs[listen_fd]);

    printf("[CoreServer] New client fd=%d from %s:%d\n",
           client_fd, peer_ip.c_str(), peer_port);
}

void CoreServer::handleClientRead(int fd)
{
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it == _clients.end()) return;
    Client* client = it->second;

    ssize_t n = client->recv();
    if (n == 0) { closeClient(fd); return; }
    if (n <  0) { closeClient(fd); return; }

    client->feedHandler();

    s_state hs = client->getHandler().getState();
    if (hs == COMPLETE || hs == ERROR)
    {
        // client->getWriteBuf() = client->getHandler().getResponseString();

        client->getHandler().reset();
    }

    if (client->wantWrite()) {
        epollMod(fd, EPOLLIN | EPOLLOUT);
        client->setState(STATE_WRITING);
    }
}

void CoreServer::handleClientWrite(int fd)
{
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it == _clients.end()) return;
    Client* client = it->second;

    ssize_t n = client->send();
    if (n < 0) { closeClient(fd); return; }

    if (!client->wantWrite()) {
        epollMod(fd, EPOLLIN);
        client->setState(STATE_READING);
    }
}

void CoreServer::closeClient(int fd)
{
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it == _clients.end()) return;

    printf("[CoreServer] Closing client fd=%d\n", fd);
    epollDel(fd); 
    delete it->second;
    _clients.erase(it);
}

bool CoreServer::epollAdd(int fd, uint32_t events)
{
    epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.events = events; ev.data.fd = fd;
    if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        perror("epoll_ctl ADD"); return false;
    }
    return true;
}

bool CoreServer::epollMod(int fd, uint32_t events)
{
    epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.events = events; ev.data.fd = fd;
    if (epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev) < 0) {
        perror("epoll_ctl MOD"); return false;
    }
    return true;
}

bool CoreServer::epollDel(int fd)
{
    epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    if (epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, &ev) < 0) {
        perror("epoll_ctl DEL"); return false;
    }
    return true;
}
