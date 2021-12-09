#include "../include/Client.h"

Client::Client(const char* addr, const int32_t& port) {
    conn.m_addr = addr;
    conn.m_port = port;
    Client::init();
}

Client::~Client() {
    close(conn.m_socket_fd);
}

void Client::init() noexcept {
    define_fd();
    add_hint();
    connect();
    LOG(Log::INFO, "You have successfully connected to the VFS: [address : " + std::string(conn.m_addr) + "]");
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
    if((inet_pton(AF_INET, conn.m_addr, &conn.hint.sin_addr)) == -1) {
        LOG(Log::ERROR_, "Address specified is not valid");
        return;
    }
}

void Client::connect() noexcept {
    if(::connect(conn.m_socket_fd, (sockaddr*)&conn.hint, sizeof(conn.hint)) == -1) {
        LOG(Log::ERROR_, "Could not connect to VFS, please try again");
        return;
    }
}

void Client::send(const char* buffer) noexcept {
    if(::send(conn.m_socket_fd, buffer, sizeof(buffer), 0) == -1) {
        LOG(Log::WARNING, "input could not be sent towards remote VFS");
        return;
    }
}

void Client::receive() noexcept {

}

void Client::run() noexcept {

}

