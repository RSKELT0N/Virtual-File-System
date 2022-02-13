#include "../include/Client.h"

extern constexpr unsigned int hash(const char *s, int off = 0);

Client::Client(const char* addr, const int32_t& port) {
    conn.m_addr = addr;
    conn.m_port = port;
    info.state = CFG_SOCK_OPEN;
    Client::init();
}

Client::~Client() {
    info.state = CFG_SOCK_CLOSE;
    close(conn.m_socket_fd);
    BUFFER << "Deleted Client\n";
}

void Client::init() noexcept {
    define_fd();
    add_hint();
    connect();
    BUFFER << LOG_str(Log::INFO, "You have successfully connected to the VFS: [address : " + std::string(conn.m_addr) + "]");
    recv_ = std::thread(&Client::run, this);
    recv_.detach();

}

void Client::define_fd() noexcept {
    if((conn.m_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        BUFFER << LOG_str(Log::ERROR_, "Socket[SOCK_STREAM] could not be created");
        return;
    }
}

void Client::add_hint() noexcept {
    conn.hint.sin_family = AF_INET;
    conn.hint.sin_port   = htons(conn.m_port);

    unsigned long res = inet_addr(conn.m_addr);
    if(res == INADDR_NONE) {
        BUFFER << LOG_str(Log::ERROR_, "Address specified is not valid");
        return;
    } else conn.hint.sin_addr = in_addr{(in_addr_t)res};
}

void Client::connect() noexcept {
    if(::connect(conn.m_socket_fd, (sockaddr*)&conn.hint, sizeof(conn.hint)) == -1) {
        BUFFER << LOG_str(Log::ERROR_, "Could not connect to VFS, please try again");
        return;
    }
}

void Client::handle_send(const char* str_cmd, uint8_t cmd, std::vector<std::string>& args) noexcept {
    std::string payload = get_payload(str_cmd, args);
    pcontainer_t* container = generate_container(cmd, args, payload);
    // Serialize packet.
    char buffer[CFG_PACKET_SIZE];
    memset(buffer, 0, CFG_PACKET_SIZE);
    serialize_packet(container->info, buffer);
    // sending info packet towards server, with command info related to input. Formatted ispl.
    send(buffer, sizeof(packet_t));

    int i;
    // checking if payload flag is set
    if(container->info.p_count) {
        // looping through payloads until mf is not equal to zero.
        for(i = 0; container->payloads->at(i).header.mf == 1; i++) {
            memset(buffer, 0, CFG_PACKET_SIZE);
            // Serialize payload
            serialize_payload(container->payloads->at(i), buffer);
            // send payload per fragment.
            send(buffer, sizeof(payload_t));
        }
        memset(buffer, 0, CFG_PACKET_SIZE);
        // Serialize payload
        serialize_payload(container->payloads->at(i), buffer);
        // send last payload with mf whichs to zero.
        send(buffer, sizeof(payload_t));
    }
    //packet has been sent.
    delete container;
}

void Client::send(const void* buffer, size_t buffer_size) noexcept {
    int number_of_bytes = {};
    size_t bytes_sent = {};

    while(number_of_bytes < buffer_size && (bytes_sent = ::send(conn.m_socket_fd, buffer, buffer_size, MSG_NOSIGNAL))) {
        number_of_bytes += bytes_sent;
        buffer += bytes_sent;

        if(bytes_sent == -1) {
            BUFFER << LOG_str(Log::WARNING, "input could not be sent towards remote VFS");
            return;
        }
    }
}

void Client::receive(size_t bytes) noexcept {
    char buffer_[bytes];
    char* buffer = (char*)buffer_;
    int number_of_bytes = {};
    size_t bytes_received = {};

    while(number_of_bytes < bytes && (bytes_received = recv(conn.m_socket_fd, buffer, bytes, MSG_NOSIGNAL)) > 0) {
        number_of_bytes += bytes_received;
        buffer += bytes_received;

        if(bytes_received == -1) {
            BUFFER << (LOG_str(Log::ERROR_, "Client was unable to read incoming data from mounted server"));
        } else if (bytes_received == 0) {
            info.state = CFG_SOCK_CLOSE;
            VFS::get_vfs()->control_vfs({"umnt"});
            BUFFER << (LOG_str(Log::SERVER, "Disconnected from server"));
        }
    }
    interpret_input(buffer_);
}

void Client::interpret_input(char* segs) noexcept {
    BUFFER.hold_buffer();

    payload_t tmp;
    deserialize_payload(tmp, segs);
    BUFFER << tmp.payload;

    if(tmp.header.mf == 0x1) {
        receive(sizeof(payload_t));
    } else {   
        const char* str = BUFFER.retain_buffer();
        printf("%s", str);
    }
    BUFFER.release_buffer();
}

void Client::run() noexcept {
    while(info.state == CFG_SOCK_OPEN) {
        receive(sizeof(payload_t));
    }
}

std::string Client::get_payload(const char* cmd, std::vector<std::string>& args) noexcept {
    std::string payload = "";

    if(strcmp(cmd, "cp") == 0) {
        if(strcmp(args[0].c_str(), "ext") == 0)
            payload = FS::get_ext_file_buffer(args[1].c_str());
    }

    return payload;
}