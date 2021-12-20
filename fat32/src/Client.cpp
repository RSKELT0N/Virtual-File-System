#include "../include/Client.h"

Client::Client(const char* addr, const int32_t& port) {
    have_recvd = 0;
    conn.m_addr = addr;
    conn.m_port = port;
    info.state = CFG_SOCK_OPEN;
    Client::init();
}

Client::~Client() {
    info.state = CFG_SOCK_CLOSE;
    close(conn.m_socket_fd);
    VFS::get_vfs()->append_buffr("Deleted Client\n");
}

void Client::init() noexcept {
    define_fd();
    add_hint();
    connect();
    LOG(Log::INFO, "You have successfully connected to the VFS: [address : " + std::string(conn.m_addr) + "]");
    recv_ = std::thread(&Client::run, this);
    recv_.detach();

}

void Client::define_fd() noexcept {
    if((conn.m_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        LOG(Log::ERROR_, "Socket[SOCK_STREAM] could not be created");
        return;
    }
}

void Client::add_hint() noexcept {
    conn.hint.sin_family = AF_INET;
    conn.hint.sin_port   = htons(conn.m_port);

    unsigned long res = inet_addr(conn.m_addr);
    if(res == INADDR_NONE) {
        LOG(Log::ERROR_, "Address specified is not valid");
        return;
    } else conn.hint.sin_addr = in_addr{(in_addr_t)res};
}

void Client::connect() noexcept {
    if(::connect(conn.m_socket_fd, (sockaddr*)&conn.hint, sizeof(conn.hint)) == -1) {
        LOG(Log::ERROR_, "Could not connect to VFS, please try again");
        return;
    }
}

void Client::handle_send(uint8_t cmd, std::vector<std::string>& args, const char* payload) noexcept {
    pcontainer_t* container = generate_container(cmd, args, payload);

    // sending info packet towards server, with command info related to input. Formatted ispl.
    send(&container->info, sizeof(info_t));

    int i;
    // checking if payload flag is set
    if(container->info.ispl) {
        // looping through payloads until mf is not equal to zero.
        for(i = 0; container->payloads[i].mf == 1; i++) {
            // send payload per fragment.
            send(&container->payloads[i], sizeof(container->payloads[i]));
        }
        // send last payload with mf whichs to zero.
        send(&container->payloads[i], sizeof(container->payloads[i]));
    }
    //packet has been sent.
    delete container;
}

void Client::send(const void* buffer, size_t buffer_size) noexcept {
    if(::send(conn.m_socket_fd, buffer, buffer_size, 0) == -1) {
        LOG(Log::WARNING, "input could not be sent towards remote VFS");
        return;
    }
}

void Client::receive() noexcept {
    char buffer[BUFFER_SIZE];
    int val = 0;

    if((val = recv(conn.m_socket_fd, buffer, BUFFER_SIZE, 0)) == -1) {
        LOG(Log::ERROR_, "Client was unable to read incoming data from mounted server");
    } else if (val == 0) {
        VFS::get_vfs()->control_vfs({"umnt"});
        LOG(Log::SERVER, "Disconnected from server");
    } else if(val > 0) {
        interpret_input(buffer);
    }
}

void Client::interpret_input(char* segs) noexcept {
    VFS::get_vfs()->append_buffr(std::string(segs));
}

void Client::run() noexcept {
    while(info.state == CFG_SOCK_OPEN) {
        receive();
    }
}