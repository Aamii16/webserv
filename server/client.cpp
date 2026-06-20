#include "client.hpp"

static const size_t RECV_CHUNK = 4096;

Client::Client(int fd, const std::string& ip, int port, t_server& server)
    : _fd(fd)
    , _ip(ip)
    , _port(port)
    , _state(STATE_READING)
    , _server(server)
    , _handler(fd)
{}

Client::~Client()
{
    if (_fd >= 0)
        close(_fd);
}

int         Client::getFd()    const { return _fd; }
std::string Client::getIp()    const { return _ip; }
int         Client::getPort()  const { return _port; }
ClientState Client::getState() const { return _state; }
void        Client::setState(ClientState s) { _state = s; }

std::string& Client::getReadBuf()  { return _readBuf; }
std::string& Client::getWriteBuf() { return _writeBuf; }
bool         Client::wantWrite()   const { return !_writeBuf.empty(); }

Handler& Client::getHandler() { return _handler; }

ssize_t Client::recv()
{
    char    buf[RECV_CHUNK];
    ssize_t n = ::recv(_fd, buf, sizeof(buf), 0);
    if (n > 0)
        _readBuf.append(buf, static_cast<size_t>(n));
    return n;
}

ssize_t Client::send()
{
    if (_writeBuf.empty())
        return 0;
    ssize_t n = ::send(_fd, _writeBuf.data(), _writeBuf.size(), 0);
    if (n > 0)
        _writeBuf.erase(0, static_cast<size_t>(n));
    return n;
}

void Client::feedHandler()
{
    _handler.process(_server, _readBuf);
    _readBuf.clear();
}
