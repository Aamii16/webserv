#include "core_server.hpp"
#include <sys/wait.h>
#include <csignal>
#include <cerrno>



CoreServer::CoreServer() : _epfd(-1)
{
// EPOLL_CLOEXEC so epoll fd don't leak into CGI child processes
    _epfd = epoll_create1(EPOLL_CLOEXEC); 
    if (_epfd < 0)
        perror("epoll_create1");
}

CoreServer::~CoreServer()
{
    for (std::map<int, CgiState*>::iterator it = _cgiByClient.begin();
         it != _cgiByClient.end(); ++it)
    {
        CGIHandler *cgi = it->second->cgi;
        if (cgi->getPid() > 0)
            kill(cgi->getPid(), SIGKILL);
        cgi->closeStdin();
        cgi->closeStdout();
        int status;
        cgi->reap(status);
        delete it->second;
    }
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
        // Bounded timeout (instead of -1)
        int nready = epoll_wait(_epfd, events, MAX_EVENTS, 1000);
        if (nready < 0)
        {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nready; ++i)
        {
            int fd = events[i].data.fd;

            if (_configs.count(fd)) {
                handleNewConnection(fd);
                continue;
            }
            if (_cgiByFd.count(fd)) {
                handleCgiEvent(fd, events[i].events);
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

        checkCgiTimeouts();
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
    if (n <  0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return;
        closeClient(fd);
        return;
    }

    client->feedHandler();

    Handler &handler = client->getHandler();
    if (handler.isCgiPending())
    {
        registerCgi(fd, handler.getCgi());
        return;
    }

    s_state hs = handler.getState();
    if (hs == COMPLETE || hs == ERROR)
    {
        client->getWriteBuf() = handler.getResponseString();
        handler.reset();
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
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return;
        closeClient(fd);
        return;
    }

    if (!client->wantWrite()) {
        epollMod(fd, EPOLLIN);
        client->setState(STATE_READING);
    }
}

void CoreServer::closeClient(int fd)
{
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it == _clients.end()) return;

    // The client is gone if it had a CGI still running, kill it too
    std::map<int, CgiState*>::iterator cgiIt = _cgiByClient.find(fd);
    if (cgiIt != _cgiByClient.end())
        dropCgiState(cgiIt->second, true);

    printf("[CoreServer] Closing client fd=%d\n", fd);
    epollDel(fd); 
    delete it->second;
    _clients.erase(it);
}

//CGI / epoll integration

void CoreServer::registerCgi(int client_fd, CGIHandler *cgi)
{
    CgiState *st = new CgiState(cgi, client_fd);
    _cgiByClient[client_fd] = st;

    if (cgi->getStdoutFd() >= 0)
    {
        _cgiByFd[cgi->getStdoutFd()] = st;
        epollAdd(cgi->getStdoutFd(), EPOLLIN);
    }
    if (cgi->hasBodyToWrite() && cgi->getStdinFd() >= 0)
    {
        _cgiByFd[cgi->getStdinFd()] = st;
        epollAdd(cgi->getStdinFd(), EPOLLOUT);
    }
}

void CoreServer::handleCgiEvent(int fd, uint32_t events)
{
    std::map<int, CgiState*>::iterator it = _cgiByFd.find(fd);
    if (it == _cgiByFd.end()) return;
    CgiState   *st  = it->second;
    CGIHandler *cgi = st->cgi;

    if (fd == cgi->getStdinFd() && (events & (EPOLLOUT | EPOLLERR | EPOLLHUP)))
    {
        int n = cgi->writeStdin();
        if (n == -1 || !cgi->hasBodyToWrite())
        {
            epollDel(fd);
            _cgiByFd.erase(it);
            cgi->closeStdin();
        }
        return;
    }

    if (fd == cgi->getStdoutFd() && (events & (EPOLLIN | EPOLLHUP | EPOLLERR)))
    {
        int n = cgi->readStdout();
        if (n > 0)
            return; // more output may still be coming
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK) &&
            !(events & (EPOLLHUP | EPOLLERR)))
            return;

        // EOF (n == 0), a real read error
        epollDel(fd);
        _cgiByFd.erase(it);
        cgi->closeStdout();
        finishCgi(st);
    }
}

void CoreServer::finishCgi(CgiState *st)
{
    CGIHandler *cgi = st->cgi;

    if (cgi->getStdinFd() >= 0)
    {
        _cgiByFd.erase(cgi->getStdinFd());
        epollDel(cgi->getStdinFd());
        cgi->closeStdin();
    }
    if (cgi->getStdoutFd() >= 0)
    {
        _cgiByFd.erase(cgi->getStdoutFd());
        epollDel(cgi->getStdoutFd());
        cgi->closeStdout();
    }

    int exit_status = 0;
    cgi->reap(exit_status);

    std::map<int, Client*>::iterator cit = _clients.find(st->client_fd);
    if (cit != _clients.end())
    {
        Client  *client  = cit->second;
        Handler &handler = client->getHandler();

        if (!WIFEXITED(exit_status) || WEXITSTATUS(exit_status) != 0)
            handler.abortCgi(client->getServer(), INTERNAL_SERVER_ERROR);
        else
            handler.finishCgi(client->getServer());

        client->getWriteBuf() = handler.getResponseString();
        handler.reset();

        if (client->wantWrite()) {
            epollMod(client->getFd(), EPOLLIN | EPOLLOUT);
            client->setState(STATE_WRITING);
        }
    }

    _cgiByClient.erase(st->client_fd);
    delete st;
}

void CoreServer::checkCgiTimeouts()
{
    time_t now = time(NULL);
    std::vector<CgiState*> timedOut;

    for (std::map<int, CgiState*>::iterator it = _cgiByClient.begin();
         it != _cgiByClient.end(); ++it)
        if (now - it->second->start_time >= CGI_TIMEOUT_SECONDS)
            timedOut.push_back(it->second);

    for (size_t i = 0; i < timedOut.size(); ++i)
    {
        CgiState   *st  = timedOut[i];
        CGIHandler *cgi = st->cgi;

        printf("[CoreServer] CGI timed out (client fd=%d), killing pid=%d\n",
               st->client_fd, static_cast<int>(cgi->getPid()));

        if (cgi->getPid() > 0)
            kill(cgi->getPid(), SIGKILL);

        if (cgi->getStdinFd() >= 0)  { _cgiByFd.erase(cgi->getStdinFd());  epollDel(cgi->getStdinFd());  cgi->closeStdin();  }
        if (cgi->getStdoutFd() >= 0) { _cgiByFd.erase(cgi->getStdoutFd()); epollDel(cgi->getStdoutFd()); cgi->closeStdout(); }

        int exit_status = 0;
        cgi->reap(exit_status); // child is dying/dead, waitpid returns promptly

        std::map<int, Client*>::iterator cit = _clients.find(st->client_fd);
        if (cit != _clients.end())
        {
            Client  *client  = cit->second;
            Handler &handler = client->getHandler();

            handler.abortCgi(client->getServer(), GATEWAY_TIMEOUT);
            client->getWriteBuf() = handler.getResponseString();
            handler.reset();

            if (client->wantWrite()) {
                epollMod(client->getFd(), EPOLLIN | EPOLLOUT);
                client->setState(STATE_WRITING);
            }
        }

        _cgiByClient.erase(st->client_fd);
        delete st;
    }
}

void CoreServer::dropCgiState(CgiState *st, bool killChild)
{
    CGIHandler *cgi = st->cgi;

    if (killChild && cgi->getPid() > 0)
        kill(cgi->getPid(), SIGKILL);

    if (cgi->getStdinFd() >= 0)  { _cgiByFd.erase(cgi->getStdinFd());  epollDel(cgi->getStdinFd());  cgi->closeStdin();  }
    if (cgi->getStdoutFd() >= 0) { _cgiByFd.erase(cgi->getStdoutFd()); epollDel(cgi->getStdoutFd()); cgi->closeStdout(); }

    int exit_status = 0;
    cgi->reap(exit_status);

    _cgiByClient.erase(st->client_fd);
    delete st;
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
