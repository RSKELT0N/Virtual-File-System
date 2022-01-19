#ifndef _SERVER_H_
#define _SERVER_H_

#include <vector>
#include <thread>
#include <utility>

#include "VFS.h"
#include "RFS.h"

#ifndef _WIN32
    #include <netinet/in.h>
    #include <unistd.h>
#else
    #include <winsock2.h>
    #include <ws2tcpip.h>
#endif


#define SOCK_OPEN             (uint8_t)1
#define SOCK_CLOSE            (uint8_t)0
#define EMPH_START            (uint32_t)49152
#define EMPH_END              (uint32_t)65535
#define PORT_RANGE(__port__)  (__port__ < EMPH_START || __port__ >  EMPH_END) ? 1 : 0
#define SERVER_REPONSE        (const char*)""

class Server : public RFS {

private:
	struct connect_t {
		uint32_t m_port;
		int m_socket_fd;
		sockaddr_in hint;
		int opt = 1;
	}conn;

    struct info_t {
        uint8_t state;
        uint8_t users_c;
        uint8_t max_usr_c;
    }info;

    struct client_t {
        int sock_fd;
        sockaddr_in hint;
        const char* ip;
        std::thread* thrd;
        ~client_t() {if(thrd) delete thrd; close(sock_fd);}
    };

public:
    Server();
    ~Server();
    Server(const Server&) = delete;
    Server(Server&&)      = delete;

private:
    void init() noexcept override;
    void define_fd() noexcept override;

public:
    void run() noexcept override;
    void receive(client_t&) noexcept;
    void send(const char* buffer, client_t&) noexcept;
    void recv_(char* buffer, client_t&) noexcept;

private:
    void set_sockopt() noexcept;
    void bind_sock() noexcept;
    void mark_listener() noexcept;
    void add_client(const uint32_t& sock, const sockaddr_in& hint) noexcept;

private:
    void handle(client_t*) noexcept;
    std::string find_ip(const sockaddr_in& sock) const noexcept;

public:
    const char* print_logs() const noexcept;
    void interpret_input(pcontainer_t*, client_t&) noexcept;
    void set_state(const uint8_t& state) noexcept;

private:
    std::vector<std::pair<uint32_t, client_t*>>* clients;
    std::vector<const char*> logs;
};

#endif // _SERVER_H_