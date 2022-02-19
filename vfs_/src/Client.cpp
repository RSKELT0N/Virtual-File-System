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

    char** payload = (char**)malloc(sizeof(char*));
    *payload = nullptr;
    get_payload(str_cmd, args, *payload);
    pcontainer_t* container = generate_container(cmd, args, *payload);
    free(payload);
    // Serialize packet.
    char buffer[CFG_PACKET_SIZE];
    memset(buffer, 0, CFG_PACKET_SIZE);
    serialize_packet(container->info, buffer);
    // sending info packet towards server, with command info related to input. Formatted ispl.
    send(buffer, sizeof(packet_t));

    int i;
    for(i = 0; i < container->info.p_count; i++) {
        memset(buffer, 0, CFG_PACKET_SIZE);
        // Serialize payload
        serialize_payload(container->payloads->at(i), buffer);
        // send payload per fragment.
        send(buffer, sizeof(payload_t));
    }
    //packet has been sent.
    delete container;
}

void Client::send(const char* buffer, size_t buffer_size) noexcept {
    int number_of_bytes = {};
    size_t bytes_sent = {};

    while(number_of_bytes < buffer_size && (bytes_sent = ::send(conn.m_socket_fd, buffer, (buffer_size - number_of_bytes), MSG_NOSIGNAL))) {
        number_of_bytes += bytes_sent;
        buffer += bytes_sent;

        if(bytes_sent == -1) {
            BUFFER << LOG_str(Log::WARNING, "input could not be sent towards remote VFS");
            return;
        }
    }
}

uint8_t Client::receive(char* buffer, size_t bytes) noexcept {
    int number_of_bytes = {};
    size_t bytes_received = {};

    while(number_of_bytes < bytes && (bytes_received = recv(conn.m_socket_fd, buffer, (bytes - number_of_bytes), MSG_NOSIGNAL)) > 0) {
        number_of_bytes += bytes_received;
        buffer += bytes_received;

        if(bytes_received == -1) {
            BUFFER << (LOG_str(Log::WARNING, "Client was unable to read incoming data from mounted server"));
            return -1;
        } else if (bytes_received == 0) {
            info.state = CFG_SOCK_CLOSE;
            VFS::get_vfs()->control_vfs({"umnt"});
            BUFFER << (LOG_str(Log::SERVER, "Disconnected from server"));
            return 0;
        }
    }
    return 1;
}

void Client::receive_from_server() noexcept {
    char buffer[CFG_PACKET_SIZE];
    memset(buffer, 0, CFG_PACKET_SIZE);

    if(receive(buffer, sizeof(packet_t)) < 1)
        return;
        
    pcontainer_t* container = new pcontainer_t;
    deserialize_packet(container->info, buffer);

    if(process_packet(container->info) == 0) {
        ::send(conn.m_socket_fd, CFG_INVALID_PROTOCOL, strlen(CFG_INVALID_PROTOCOL), 0);
        info.state = CFG_SOCK_CLOSE;
        delete container;
        return;
    }
    
    payload_t tmp_payload;
    for(int i = 0; i < container->info.p_count; i++) {
        memset(buffer, 0, CFG_PACKET_SIZE);
        receive(buffer, sizeof(payload_t));
        deserialize_payload(tmp_payload, buffer);
        
        if(process_payload(container->info, tmp_payload) == 0) {
            ::send(conn.m_socket_fd, CFG_INVALID_PROTOCOL, strlen(CFG_INVALID_PROTOCOL), 0);
            info.state = CFG_SOCK_CLOSE;
            delete container;
            return;
        }

        container->payloads->push_back(tmp_payload);
    }

    interpret_input(*container);
    delete container;
}

void Client::interpret_input(const pcontainer_t& container) noexcept {
    BUFFER.hold_buffer();

    uint64_t payload_size = 0;

    for(int i = 0; i < container.info.p_count; i++)
        payload_size += container.payloads->at(i).header.size;

    char* buffer = (char*)malloc(sizeof(char) * payload_size);
    memset(buffer, 0, payload_size);

    uint64_t data_copied = 0;
    for(int i = 0; i < container.info.p_count; i++) {
        memcpy(&(buffer[data_copied]), container.payloads->at(i).payload, container.payloads->at(i).header.size);
        data_copied += container.payloads->at(i).header.size;
    }

    buffer[data_copied] = '\0';
    BUFFER << buffer;
    memset(buffer, 0, payload_size);
    free(buffer);

    const char* stream = BUFFER.retain_buffer();
    printf("%s\n", stream);
    BUFFER.release_buffer();
}

void Client::run() noexcept {
    while(info.state == CFG_SOCK_OPEN) {
        receive_from_server();
    }
}

void Client::get_payload(const char* cmd, std::vector<std::string>& args, char*& payload) noexcept {
    if(strcmp(cmd, "cp") == 0) {
        if(strcmp(args[0].c_str(), "imp") == 0)
            FS::get_ext_file_buffer(args[1].c_str(), payload);
    }

    if(payload == nullptr) {
        payload = (char*)malloc(sizeof(char));
        *payload = '\0';
    }
}