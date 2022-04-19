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

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

class Client : public RFS {
    
public:
    struct info_t {
        uint8_t state = {};
    }info;
    
    struct connect_t {
        int32_t m_port = {};
        const char* m_addr = {};
        int m_socket_fd = {};
        sockaddr_in hint = {};
    }conn;

public:
    Client(const char* addr, const int32_t& port);

    ~Client();
    Client(Client&&) = delete;
    Client(const Client&) = delete;

public:
    void receive_from_server() noexcept;
    uint8_t receive(char* buffer, size_t bytes) noexcept;
    void send(const char*, size_t) noexcept;
    void get_payload(const char*, std::vector<std::string>&, char*&) noexcept;
    void handle_send(const char*, uint8_t, std::vector<std::string>&) noexcept;

private:
    void add_hint() noexcept;
    void run() noexcept override;
    void init() noexcept override;
    void define_fd() noexcept override;

private:
    void connect() noexcept;
    void interpret_input(const pcontainer_t&) noexcept;

private:
    std::thread recv_;
};

#endif // _CLIENT_H_