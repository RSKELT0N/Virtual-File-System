#ifndef _RFS_H_
#define _RFS_H_

#include <stdlib.h>

#include "FS.h"
#include "Log.h"
#include "config.h"

class RFS : public FS {
public:
    RFS();
    ~RFS();
    RFS(const RFS&) = delete;
    RFS(RFS&&) = delete;

public:
    __attribute__((unused)) virtual void init() noexcept = 0;
    __attribute__((unused)) virtual void define_fd() noexcept = 0;
    __attribute__((unused)) virtual void run() noexcept = 0;
    __attribute__((unused)) virtual void receive() noexcept = 0;
    __attribute__((unused)) virtual void send(const char* buffer) noexcept = 0;
};

#endif // _RFS_H_