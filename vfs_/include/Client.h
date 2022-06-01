#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <unistd.h>
#include <thread>

#include "VFS.h"
#include "RFS.h"
#include "lib.h"
#include "Buffer.h"

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
    uint64_t get_payload(const char*, std::vector<std::string>&, std::byte*&) noexcept;
    void handle_send(const char*, uint8_t, std::vector<std::string>&) noexcept;

private:
    void add_hint() noexcept;
    void run() noexcept override;
    void init() noexcept override;
    void define_fd() noexcept override;

private:
    void connect() noexcept;
    void interpret_input(const pcontainer_t&) noexcept;
    void ping() noexcept;
    void ping_server() noexcept;
    void set_recieved_ping(int8_t) noexcept;
    void set_state(int8_t) noexcept;

private:
    std::thread recv_;
    std::thread ping_;
    int8_t recieved_ping;

};

#endif // _CLIENT_H_