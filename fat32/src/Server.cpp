#include "../include/Server.h"

RFS::RFS() {}
RFS::~RFS() {}

Server::Server() {
    if(PORT_RANGE(CFG_DEFAULT_PORT)) {
        LOG(Log::WARNING, "Port specified is out of range, must be between (49151 - 65536)");
        return;
    }

    clients = new std::vector<std::pair<uint32_t, client_t*>>();
    conn.m_port     = CFG_DEFAULT_PORT;
    info.state      = CFG_SOCK_OPEN;
    info.max_usr_c  = CFG_SOCK_LISTEN_AMT;
    init();
}

Server::~Server() {
    info.state = CFG_SOCK_CLOSE;

    close(conn.m_socket_fd);
    delete clients;
    LOG(Log::INFO, "Socket has been closed off for conntection");
}

void Server::init() noexcept {
    printf("\n%s\n", "-------  Server  ------");
    define_fd();
    set_sockopt();
    bind_sock();
    mark_listener();

    char str[10];
    sprintf(str, "%d", conn.m_port);
    LOG(Log::SERVER, "Server has been initialised: [Port : " + std::string(str) + "]");
    printf("%s\n", "---------  End  -------");
    std::thread run_(&Server::run, this);
    run_.detach();
}

void Server::define_fd() noexcept {
    if((conn.m_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        LOG(Log::ERROR_, "Socket couldnt be created");
    } else LOG(Log::SERVER, "Socket[Socket Stream] has been created");
}

void Server::set_sockopt() noexcept {
    if(setsockopt(conn.m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &conn.opt, sizeof(conn.opt)) == -1) {
        LOG(Log::ERROR_, "Issue setting the Socket level to: SOL_SOCKET");
    } else LOG(Log::SERVER, "Socket level has been set towards: SOL_SOCKET");
}

void Server::bind_sock() noexcept {
    conn.hint.sin_family      = AF_INET;
    conn.hint.sin_addr.s_addr = INADDR_ANY;
    conn.hint.sin_port        = htons(conn.m_port);
    conn.sockSize             = sizeof(conn.hint);

    if(bind(conn.m_socket_fd, (sockaddr*)&conn.hint, conn.sockSize) ==  -1) {
        LOG(Log::ERROR_, "Issue binding socket to hint structure");
    } else LOG(Log::SERVER, "Socket has been bound to hint structure");
}

void Server::mark_listener() noexcept {
    if(listen(conn.m_socket_fd, CFG_SOCK_LISTEN_AMT) == -1) {
        LOG(Log::ERROR_, "Socket failed to set listener");
    } else LOG(Log::SERVER, "Socket is set for listening");
}

void Server::run() noexcept {
    uint32_t socket;
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);

    while(info.state == CFG_SOCK_OPEN) {
        if((socket = accept(conn.m_socket_fd,(sockaddr *)&client,&clientSize)) == -1) {
            LOG(Log::WARNING, "Client socket has failed to join");
            continue;
        }

        if(info.users_c == info.max_usr_c) {
            LOG(Log::WARNING, "Client: [" + find_ip(client) + "] has tried to join, Server is full");
            close(socket);
            continue;
        }

        info.users_c++;
        add_client(socket, client);
        printf("Joined\n");
    }
}

void Server::add_client(const uint32_t& sock, const sockaddr_in& hnt) noexcept {
    client_t* tmp = (client_t*)malloc(sizeof(client_t));
    tmp->sock_fd = (int)sock;
    tmp->hint = hnt;
    tmp->ip = find_ip(tmp->hint).c_str();

    clients->push_back(std::make_pair(clients->size(), tmp));
    tmp->thrd = new std::thread(&Server::handle, this);
    tmp->thrd->detach();
}

void Server::handle() noexcept {
    while(info.state == CFG_SOCK_OPEN) {
        
    }
}

void Server::send(const char* buffer) noexcept {

}

void Server::receive() noexcept {

}

std::string Server::find_ip(const sockaddr_in& sock) const noexcept {
    unsigned long addr = sock.sin_addr.s_addr;
    char buffer[50];
    sprintf(buffer, "%d.%d.%d.%d",
            (int)(addr  & 0xff),
            (int)((addr & 0xff00) >> 8),
            (int)((addr & 0xff0000) >> 16),
            (int)((addr & 0xff000000) >> 24));
    return std::string(buffer);
}




