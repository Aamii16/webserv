#pragma once

#include <string>
#include <sys/types.h>
#include "Handler.hpp"
#include <unistd.h>
#include <sys/socket.h>

enum ClientState {
    STATE_READING,
    STATE_WRITING,
    STATE_CLOSING
};

class Client {
public:
    Client(int fd, const std::string& ip, int port, t_server& server);
    ~Client();

    int         getFd()    const;
    std::string getIp()    const;
    int         getPort()  const;
    ClientState getState() const;
    void        setState(ClientState s);

    std::string& getReadBuf();
    std::string& getWriteBuf();
    bool         wantWrite() const;

    ssize_t recv();

    ssize_t send();

    void     feedHandler();

    Handler& getHandler();

private:
    int          _fd;
    std::string  _ip;
    int          _port;
    ClientState  _state;
    t_server&    _server;
    Handler      _handler;

    std::string  _readBuf;
    std::string  _writeBuf;

    Client(const Client&);
    Client& operator=(const Client&);
};
