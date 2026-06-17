#pragma once

#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>

enum ClientState {
    STATE_READING,
    STATE_WRITING,
    STATE_CLOSING
};

class Client {
public:
    Client(int fd, const std::string& ip, int port);
    ~Client();

    int         getFd()    const;
    std::string getIp()    const;
    int         getPort()  const;
    ClientState getState() const;

    void setState(ClientState state);

    std::string& getReadBuf();
    std::string& getWriteBuf();

    ssize_t recv(); // Append
    ssize_t send();

    bool wantWrite() const;

private:
    int         _fd;
    std::string _ip;
    int         _port;
    ClientState _state;

    std::string _readBuf;
    std::string _writeBuf;

    Client(const Client&);
    Client& operator=(const Client&);
};
