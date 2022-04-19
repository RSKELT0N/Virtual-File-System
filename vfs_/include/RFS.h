#ifndef _RFS_H_
#define _RFS_H_

#include <thread>
#include <string.h>
#include <stdlib.h>
#include <vector>

#include "config.h"
#include "FS.h"

class RFS : public FS {

protected:
    struct packet_t {
        char signature[CFG_PACKET_SIGNATURE_SIZE];
        uint64_t size = {};
        uint64_t p_count = {};
        uint8_t cmd = {};
        char hash[CFG_PACKET_HASH_SIZE];
        char flags[CFG_FLAGS_BUFFER_SIZE];
    } __attribute__((packed));

    struct payload_header_t {
        char hash[CFG_PACKET_HASH_SIZE];
        uint64_t index;
        uint64_t size;
        uint8_t mf : 1;
    } __attribute__((packed));

    struct payload_t {
        payload_header_t header;
        char payload[CFG_PAYLOAD_SIZE];
    } __attribute__((packed));

    struct pcontainer_t {
        packet_t info;
        std::vector<payload_t>* payloads  = new std::vector<payload_t>();

        ~pcontainer_t() {delete payloads;}
    };

public:
    RFS();
    ~RFS();
    RFS(const RFS&) = delete;
    RFS(RFS&&) = delete;

public:
    __attribute__((unused)) virtual void init() noexcept = 0;
    __attribute__((unused)) virtual void run() noexcept = 0;
    __attribute__((unused)) virtual void define_fd() noexcept = 0;


protected:
    void serialize_packet(packet_t&, char*) noexcept;
    void deserialize_packet(packet_t&, char*) noexcept;
    uint8_t process_packet(const packet_t&) const noexcept;

    void serialize_payload(payload_t&, char*) noexcept;
    void deserialize_payload(payload_t&, char*) noexcept;
    uint8_t process_payload(const packet_t&, const payload_t&) const noexcept;

    void print_packet(const packet_t&) const noexcept;
    void print_payload(const payload_t&) const noexcept;

public:
    void retain_payloads(char*& buffer, std::vector<payload_t>&) noexcept;
    pcontainer_t* generate_container(uint8_t cmd, std::vector<std::string>& args, char*& payload) noexcept;

public:
    constexpr static size_t BUFFER_SIZE = sizeof(packet_t) + sizeof(payload_t);
};

#endif // _RFS_H_
