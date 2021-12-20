#ifndef _RFS_H_
#define _RFS_H_

#include <thread>
#include <string.h>
#include <stdlib.h>
#include <vector>

#include "FS.h"
#include "Log.h"
#include "config.h"

#define PAYLOAD_SIZE 1000

class RFS : public FS {

protected:
    struct packet_t {
        
        uint8_t cmd : 1;
        std::string flags;
        uint8_t ispl : 1;
    } __attribute__((packed));

    struct payload_t {
        uint8_t mf : 1;
        char payload[PAYLOAD_SIZE];
    } __attribute__((packed));

    struct pcontainer_t {
        packet_t info;
        std::vector<payload_t> payloads;
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

public:
    pcontainer_t* generate_container(uint8_t cmd, std::vector<std::string>& args, const char* payload) noexcept;
};

#endif // _RFS_H_
