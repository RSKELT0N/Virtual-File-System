#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <unistd.h>
#include <thread>

#include "VFS.h"
#include "RFS.h"

#ifndef _WIN32
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
#else
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#endif


class Client : public RFS {
public:

    struct info_t {
        uint8_t state;
    }info;
    
    struct connect_t {
        int32_t m_port;
        const char* m_addr;
        int m_socket_fd;
        sockaddr_in hint;
    }conn;

public:
    Client(const char* addr, const int32_t& port);

    ~Client();
    Client(Client&&) = delete;
    Client(const Client&) = delete;

public:
    void receive() noexcept;
    void send(const void*, size_t) noexcept;
    void handle_send(uint8_t cmd, std::vector<std::string>& args, const char* payload) noexcept;

private:
    void add_hint() noexcept;
    void run() noexcept override;
    void init() noexcept override;
    void define_fd() noexcept override;

private:
    void connect() noexcept;
    void interpret_input(char*) noexcept;

private:
    std::thread recv_;
    bool have_recvd;
     
};

#endif // _CLIENT_H_