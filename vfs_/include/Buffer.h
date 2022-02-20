#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <mutex>
#include <stdio.h>
#include <string>

#include "config.h"

class Buffer {
   
private:
    Buffer();
    Buffer(Buffer&&) = delete;
    Buffer(const Buffer&);
    
public:
    ~Buffer();


public:
    static Buffer* get_buffer() noexcept;
    Buffer& operator<<(uint64_t) noexcept;
    Buffer& operator<<(const char*) noexcept;

    void hold_buffer() noexcept;
    void release_buffer() noexcept;
    const char* retain_buffer() noexcept;
   
private:
    std::mutex* mLock;
    static Buffer* mBuf_p;
public:
    std::string *mStream;
};


#endif // _BUFFER_H_
