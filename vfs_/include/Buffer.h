#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <mutex>
#include <stdio.h>
#include <cstring>

#include "config.h"
#include "lib.h"

#define BUFFER     (*Buffer::get_buffer())

class Buffer {
   
private:
    Buffer();
    Buffer(Buffer&&) = delete;
    Buffer(const Buffer&) = delete;
    
public:
    ~Buffer();

public:
    static Buffer* get_buffer() noexcept;
    Buffer& operator<<(uint64_t) noexcept;
    Buffer& operator<<(const char*) noexcept;

    void hold_buffer() noexcept;
    void release_buffer() noexcept;
    void retain_buffer(char*& store) noexcept;
    void retain_buffer(std::byte*&) noexcept;
    void print_stream() noexcept;
    void append(char*, size_t) noexcept;
   
private:
    std::mutex* mLock;
    static Buffer* mBuf_p;
public:
    std::vector<char>* mStream;
};


#endif // _BUFFER_H_
