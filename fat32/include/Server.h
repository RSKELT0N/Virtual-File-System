#ifndef _SERVER_H_
#define _SERVER_H_

#include "RFS.h"
#include "Log.h"

#include <vector>
#include <thread>
#include <utility>

#ifdef LINUX__
    #include <netinet/in.h>
    #include <unistd.h>
#else
    #include <winsock2.h>
#endif


#define SOCK_OPEN             (uint8_t)1
#define SOCK_CLOSE            (uint8_t)0
#define EMPH_START            (uint32_t)49152
#define EMPH_END              (uint32_t)65535
#define PORT_RANGE(__port__)  (__port__ < EMPH_START || __port__ >  EMPH_END) ? 1 : 0

class Server : public RFS {

private:
	struct connect_t {
		uint32_t m_port;
		int m_socket_fd;
		sockaddr_in hint;
		socklen_t sockSize;
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
    void receive() noexcept override;
    void send(const char* buffer) noexcept override;

public:
    void run() noexcept override;
    void set_state(const uint8_t& state) noexcept;

private:
    void define_fd()      noexcept override;
    void set_sockopt()    noexcept;
    void bind_sock()      noexcept;
    void mark_listener()  noexcept;
    void handle()         noexcept;
    void add_client(const uint32_t& sock, const sockaddr_in& hint) noexcept;

private:
    std::string find_ip(const sockaddr_in& sock) const noexcept;

private:
    std::vector<std::pair<uint32_t, client_t*>>* clients;

};

#endif // _SERVER_H_