#include "../include/Client.h"

Client::Client(const char* addr, const int32_t& port) {
    conn.m_addr = addr;
    conn.m_port = port;
    set_state(CFG_SOCK_OPEN);
    set_recieved_ping(0);
    Client::init();
}

Client::~Client() {
    set_state(CFG_SOCK_CLOSE);
}

void Client::init() noexcept {
    define_fd();
    add_hint();
    connect();
    BUFFER << LOG_str(Log::INFO, "You have successfully connected to the VFS: [address : " + std::string(conn.m_addr) + "]");
    recv_ = std::thread(&Client::run, this);
    recv_.detach();

    ping_ = std::thread(&Client::ping, this);
    ping_.detach();
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
        LOG(Log::ERROR_, "Could not connect to VFS, please try again or check IP/ Port specified. Make sure server, is initialised");
    }
}

void Client::run() noexcept {
    while(info.state == CFG_SOCK_OPEN) {
        receive_from_server();
    }
    close(conn.m_socket_fd);
    VFS::get_vfs()->control_vfs({"umnt"});
    BUFFER << (LOG_str(Log::SERVER, "Disconnected from server, either it has crashed or closed off it's connection"));
}

void Client::handle_send(const char* str_cmd, uint8_t cmd, std::vector<std::string>& args) noexcept {
    std::byte* payload = nullptr;
    uint64_t size = get_payload(str_cmd, args, payload);
    pcontainer_t* container = generate_container(cmd, args, payload, size);
    delete payload;

    // Serialize packet.
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    serialize_packet(container->info, buffer);

    // sending info packet towards server, with command info related to input. Formatted ispl.
    send(buffer, sizeof(packet_t));
    double data_sent = 0;
    double data_size = container->info.size;
    data_size -= (sizeof(packet_t) * 8);

    for(int i = 0; i < container->info.p_count; i++) {
        memset(buffer, 0, BUFFER_SIZE);
        // Serialize payload
        serialize_payload(container->payloads->at(i), buffer);
        // send payload per fragment.
        send(buffer, sizeof(payload_t));
        data_sent += container->payloads->at(i).header.size;

        lib_::printProgress((double)data_sent/data_size);
    }
    if(data_sent != 0) {
        lib_::printProgress((double)data_sent/data_size);
        printf("\n");
    }

    //packet has been sent.
    delete container;
}

void Client::receive_from_server() noexcept {
    fd_set master_set;
    struct timeval tv;

    FD_ZERO(&master_set);
    FD_SET(conn.m_socket_fd, &master_set);
    tv.tv_sec = SOCKET_ACCEPT_TIME;

    select(conn.m_socket_fd + 1, &master_set, NULL, NULL, &tv);
        
    if(FD_ISSET(conn.m_socket_fd, &master_set)) {
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);

        receive(buffer, sizeof(packet_t));
            
        pcontainer_t* container = new pcontainer_t;
        deserialize_packet(container->info, buffer);

        if(container->info.cmd == (int8_t)VFS::ping) {
            set_recieved_ping(1);
            delete container;
            return;
        }

        if(process_packet(container->info) == 0) {
            ::send(conn.m_socket_fd, CFG_INVALID_PROTOCOL, strlen(CFG_INVALID_PROTOCOL), 0);
            set_state(CFG_SOCK_CLOSE);
            delete container;
            return;
        }
        
        payload_t tmp_payload;
        for(int i = 0; i < container->info.p_count; i++) {
            memset(buffer, 0, BUFFER_SIZE);
            receive(buffer, sizeof(payload_t));
            deserialize_payload(tmp_payload, buffer);
            
            if(process_payload(container->info, tmp_payload) == 0) {
                ::send(conn.m_socket_fd, CFG_INVALID_PROTOCOL, strlen(CFG_INVALID_PROTOCOL), 0);
                set_state(CFG_SOCK_CLOSE);
                delete container;
                return;
            }

            container->payloads->push_back(tmp_payload);
        }

        interpret_input(*container);
        delete container;
    }
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
    BUFFER.append(buffer, payload_size);
    memset(buffer, 0, payload_size);
    free(buffer);

    BUFFER.print_stream();
    BUFFER.release_buffer();
}


void Client::send(const char* buffer, size_t buffer_size) noexcept {
    psend.lock();
    int number_of_bytes = {};
    size_t bytes_sent = {};

    while(number_of_bytes < buffer_size && (bytes_sent = ::send(conn.m_socket_fd, buffer, (buffer_size - number_of_bytes), MSG_NOSIGNAL))) {
        number_of_bytes += bytes_sent;
        buffer += bytes_sent;

        if(bytes_sent == -1) {
            BUFFER << LOG_str(Log::WARNING, "input could not be sent towards remote VFS");
            psend.unlock();
            return;
        }
    }
    psend.unlock();
}

uint8_t Client::receive(char* buffer, size_t bytes) noexcept {
    precieve.lock();
    int number_of_bytes = {};
    size_t bytes_received = {};

    while(number_of_bytes < bytes && (bytes_received = recv(conn.m_socket_fd, buffer, (bytes - number_of_bytes), MSG_NOSIGNAL)) > 0) {
        number_of_bytes += bytes_received;
        buffer += bytes_received;

        if(bytes_received == -1) {
            BUFFER << (LOG_str(Log::WARNING, "Client was unable to read incoming data from mounted server"));
            precieve.unlock();
            return -1;
        }
    }
    precieve.unlock();
    return 1;
}

uint64_t Client::get_payload(const char* cmd, std::vector<std::string>& args, std::byte*& payload) noexcept {
    uint64_t size = {};
    if(strcmp(cmd, "cp") == 0) {
        if(strcmp(args[0].c_str(), "imp") == 0)
            size = FS::get_ext_file_buffer(args[1].c_str(), payload);
    }

    if(payload == nullptr) {
        payload = new std::byte;
    }
    return size;
}

void Client::ping() noexcept {
    std::vector<std::string> args;
    int8_t unrecievedAmt = 0;
    while(info.state == CFG_SOCK_OPEN) {

        if(recieved_ping == 1) {
            ping_server();
            set_recieved_ping(0);
            unrecievedAmt ^= unrecievedAmt;
        } else unrecievedAmt++;

        if(unrecievedAmt == 5) {
            BUFFER << LOG_str(Log::WARNING, "Have not recieved ping from Server, disconnecting..");
            set_state(CFG_SOCK_CLOSE);
        }
        sleep(1);
    }
}

void Client::ping_server() noexcept {
    std::string ping = "PING";
    std::byte* bytes = new std::byte[ping.length()];
    std::vector<std::string> flags;

    std::memcpy(bytes, ping.data(), ping.length());

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    pcontainer_t* container = generate_container((uint8_t)VFS::system_cmd::ping, flags, bytes, ping.length());
    serialize_packet(container->info, buffer);
    send(buffer, sizeof(packet_t));
    delete bytes;
}

void Client::set_recieved_ping(int8_t val) noexcept {
    lock.lock();
    this->recieved_ping = val;
    lock.unlock();
}

void Client::set_state(int8_t val) noexcept {
    lock.lock();
    this->info.state = val;
    lock.unlock();
}
