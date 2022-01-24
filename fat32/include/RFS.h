#ifndef _RFS_H_
#define _RFS_H_

#include <thread>
#include <string.h>
#include <stdlib.h>
#include <vector>

#include "config.h"
#include "FS.h"

#define PACKET_SIZE       (size_t)1024
#define FLAGS_BUFFER_SIZE (size_t)25
#define PAYLOAD_SIZE      (size_t)1000

class RFS : public FS {

protected:
    struct packet_t {
        uint8_t cmd;
        char flags[FLAGS_BUFFER_SIZE];
        uint8_t ispl : 1;
    } __attribute__((packed));

    struct payload_t {
        uint8_t mf : 1;
        int payload_size;
        char payload[PAYLOAD_SIZE];
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

    void serialize_payload(payload_t&, char*) noexcept;
    void deserialize_payload(payload_t&, char*) noexcept;   

    void print_packet(const packet_t&) const noexcept;
    void print_payload(const payload_t&) const noexcept;

public:
    std::string retain_payloads(std::vector<payload_t>&) noexcept;
    pcontainer_t* generate_container(uint8_t cmd, std::vector<std::string>& args, std::string payload) noexcept;
};

#endif // _RFS_H_
