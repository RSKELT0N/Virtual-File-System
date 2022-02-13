#include "../include/Server.h"

extern int itoa_(int value, char *sp, int radix, int amt);
extern std::vector<std::string> split(const char* line, char sep) noexcept;

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
    printf("Deleted Server\n");
    BUFFER << LOG_str(Log::INFO, "Socket has been closed off for conntection");
}

void Server::init() noexcept {
    BUFFER << "\n-------  Server  ------\n";
    define_fd();
    set_sockopt();
    bind_sock();
    mark_listener();

    char str[10];
    sprintf(str, "%d", conn.m_port);
    BUFFER << LOG_str(Log::SERVER, "Server has been initialised: [Port : " + std::string(str) + "]");
    BUFFER << "---------  End  -------\n";
    this->run_ = std::thread(&Server::run, this);
    run_.detach();
}

void Server::define_fd() noexcept {
    if((conn.m_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        BUFFER << LOG_str(Log::ERROR_, "Socket couldnt be created");
    } else BUFFER << LOG_str(Log::SERVER, "Socket[Socket Stream] has been created");
}

void Server::set_sockopt() noexcept {
    int res = 0;

    #ifndef _WIN32
        setsockopt(conn.m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &conn.opt, sizeof(conn.opt));
    #else
        char int_str[1];
        setsockopt(conn.m_socket_fd, SOL_SOCKET, SO_REUSEADDR, ((const char*)itoa_(conn.opt, int_str, 10, 1)), sizeof(conn.opt));
    #endif

    if(res == -1) {
        BUFFER << LOG_str(Log::ERROR_, "Issue setting the Socket level to: SOL_SOCKET");
    } else BUFFER << LOG_str(Log::SERVER, "Socket level has been set towards: SOL_SOCKET");
}

void Server::bind_sock() noexcept {
    conn.hint.sin_family      = AF_INET;
    conn.hint.sin_addr.s_addr = INADDR_ANY;
    conn.hint.sin_port        = htons(conn.m_port);

    if(bind(conn.m_socket_fd, (sockaddr*)&conn.hint, sizeof(conn.hint)) ==  -1) {
        BUFFER << LOG_str(Log::ERROR_, "Issue bindbing socket to hint structure");
    } else BUFFER << LOG_str(Log::SERVER, "Socket has been bound to hint structure");
}

void Server::mark_listener() noexcept {
    if(listen(conn.m_socket_fd, CFG_SOCK_LISTEN_AMT) == -1) {
        BUFFER << LOG_str(Log::ERROR_, "Socket failed to set listener");
    } else BUFFER << LOG_str(Log::SERVER, "Socket is set for listening");
}

void Server::run() noexcept {
    uint32_t socket;
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);

    fd_set master_set;
    struct timeval tv;

    while(info.state == CFG_SOCK_OPEN) {
        FD_ZERO(&master_set);
        FD_SET(conn.m_socket_fd, &master_set);
        tv.tv_sec = SOCKET_ACCEPT_TIME;

        select(conn.m_socket_fd + 1, &master_set, NULL, NULL, &tv);
        
        if(FD_ISSET(conn.m_socket_fd, &master_set)) {
            if((socket = accept(conn.m_socket_fd,(sockaddr *)&client,&clientSize)) == -1) {
                BUFFER << LOG_str(Log::WARNING, "Client socket has failed to join");
                continue;
            }

            if(info.users_c == info.max_usr_c) {
                BUFFER << LOG_str(Log::WARNING, "Client: [" + find_ip(client) + "] has tried to join, Server is full");
                close(socket);
                continue;
            }

            info.users_c++;
            add_client(socket, client);
            BUFFER << LOG_str(Log::SERVER, "Client has joined");
        }
    }
}

void Server::add_client(const uint32_t& sock, const sockaddr_in& hint) noexcept {
    client_t* tmp = (client_t*)malloc(sizeof(client_t));
    tmp->sock_fd = (int)sock;
    tmp->hint = hint;
    tmp->ip = find_ip(tmp->hint).c_str();
    tmp->state = CFG_SOCK_OPEN;

    clients->push_back(std::make_pair(clients->size(), tmp));
    tmp->thrd = new std::thread(&Server::handle, this, (tmp));
    tmp->thrd->detach();
}

void Server::handle(client_t* client) noexcept {
    while(info.state == CFG_SOCK_OPEN && client->state == CFG_SOCK_OPEN) {
        receive(*client);
    }
}

void Server::receive(client_t& client) noexcept {
    int val = 0;
    char buffer[CFG_PACKET_SIZE];
    // declare container to store info and payload packets.
    pcontainer_t* container = new pcontainer_t;
    // recv info packet and store it within the container info address.
    recv_(buffer, client, sizeof(packet_t));
    // deserialize packet.
    deserialize_packet(container->info, buffer);
    // reset buffer array.
    memset(buffer, 0, CFG_PACKET_SIZE);
    // check whether info has any payload.
    if(container->info.p_count == 0)
        goto no_payload;

    // recv first payload.
    payload_t tmp;
    recv_(buffer, client, sizeof(payload_t));
    // deserialize payload.
    deserialize_payload(tmp, buffer);
    // push deserialized payload into container.
    container->payloads->push_back(tmp);

    // check whether first payload has any mf flag set to 0x1 and then the last 0x0.
    while(tmp.header.mf == 0x1) {
        // reset buffer array.
        memset(buffer, 0, CFG_PACKET_SIZE);
        memset(tmp.payload, 0, CFG_PAYLOAD_SIZE);
        // recv a payload
        recv_(buffer, client, sizeof(payload_t));

        deserialize_payload(tmp, buffer);
        container->payloads->push_back(tmp);
    }

    no_payload:
    interpret_input(container, client);

    delete container;
}

void Server::send(const char* buffer, client_t& client, size_t buffer_size) noexcept {
    int number_of_bytes = {};
    size_t bytes_sent = {};

    while(number_of_bytes < buffer_size && (bytes_sent = ::send(client.sock_fd, buffer, buffer_size, MSG_NOSIGNAL))) {
        number_of_bytes += bytes_sent;
        buffer += bytes_sent;


        if(bytes_sent == -1) {
            BUFFER << LOG_str(Log::ERROR_, "Issue sending information back towards client");
            return;
        }
    }
}

void Server::recv_(char* buffer, client_t& client, size_t bytes) noexcept {
    int number_of_bytes = {};
    size_t bytes_received = {};

    while(number_of_bytes < bytes && (bytes_received = recv(client.sock_fd, buffer, bytes, MSG_NOSIGNAL)) > 0) {
        number_of_bytes += bytes_received;
        buffer += bytes_received;

        if(bytes_received == -1) {
            *buffer = '\0';
            BUFFER << LOG_str(Log::ERROR_, "Issue receiving data from client");
        } else if(bytes_received == 0) {
            info.users_c--;
            client.state = CFG_SOCK_CLOSE;
            BUFFER << LOG_str(Log::SERVER, "A client has disconnected");
        }
    }
}

void Server::interpret_input(pcontainer_t* container, client_t& client) noexcept {
    BUFFER.hold_buffer();

    std::vector<std::string> args = split(container->info.flags, ' ');
    VFS::system_cmd cmd = (VFS::system_cmd)container->info.cmd;

    if(cmd == VFS::system_cmd::cp) {
        if(strcmp(args[0].c_str(), "ext") == 0) {
            args.erase(args.begin());
            args.erase(args.begin());
            cmd = VFS::system_cmd::touch;
        }
    }

    std::string* payload = new std::string();
    if(container->info.p_count != 0)
        payload = retain_payloads(*container->payloads);

    VFS* tvfs = VFS::get_vfs();

    if(tvfs->is_mnted())
        (*tvfs.*tvfs->get_mnted_system()->access)(cmd, args, payload->c_str());
    else BUFFER << LOG_str(Log::WARNING, "Server does not have a system mounted");

    send_to_client(client);

#if _DEBUG_
    print_packet(container->info);
    for(int i = 0; i < container->payloads->size(); i++) {
        print_payload(container->payloads->at(i));
    }
#endif // _DEBUG_
    delete payload;
}

void Server::send_to_client(client_t& client) noexcept {
    const char* stream = BUFFER.retain_buffer();
    size_t load_size = strlen(stream);

    if(*stream != '\0') {
        char buffer[CFG_PACKET_SIZE];
        size_t data_read = {};
        int amount_of_payloads = load_size / CFG_PAYLOAD_SIZE;

        if(load_size > CFG_PAYLOAD_SIZE)
            if(load_size % CFG_PAYLOAD_SIZE)
                amount_of_payloads++;

        payload_t tmp;
        for(int i = 0; i < (amount_of_payloads - 1); i++) {
            tmp.header.mf = 0x1;
            tmp.header.size = CFG_PAYLOAD_SIZE;
            memcpy(tmp.payload, &(stream[data_read]), CFG_PAYLOAD_SIZE);
            serialize_payload(tmp, buffer);

            send(buffer, client, sizeof(payload_t));
            memset(buffer, 0, CFG_PACKET_SIZE);
            data_read += CFG_PAYLOAD_SIZE;
        }

        size_t remaining_data = load_size - data_read;
    
        tmp.header.mf = 0x0;
        tmp.header.size = remaining_data;
        memcpy(tmp.payload, &(stream[data_read]), remaining_data);
        serialize_payload(tmp, buffer);
        send(buffer, client, sizeof(payload_t));
    } else {
        send(SERVER_REPONSE, client);
    }

    BUFFER.release_buffer();
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

const char* Server::print_logs() const noexcept {
    char tmp[1024];

    BUFFER << "\n---- LOGS ----\n";
    for(int i = 0; i < logs.size(); i++) {
        BUFFER << "[" << i << "] -> " << logs[i] << "\n";
    }
    BUFFER << "--------------\n";
    return tmp;
}