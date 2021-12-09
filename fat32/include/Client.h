#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "RFS.h"

class Client : public RFS {
public:
    struct connect_t {
        int32_t m_port;
        const char* m_addr;
        int m_socket_fd;
        sockaddr_in hint;
    }conn;

public:
    Client(const char* addr, const int32_t& port);
    Client(const Client&) = delete;
    Client(Client&&)      = delete;
    ~Client();

public:
    void send(const char* buffer) noexcept override;

private:
    void init() noexcept override;
    void define_fd() noexcept override;
    void add_hint() noexcept;

    void connect() noexcept;
    void run() noexcept override;
    void receive() noexcept override;

};

#endif // _CLIENT_H_