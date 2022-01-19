#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <mutex>
#include <stdio.h>
#include <string>

class Buffer {
   
private:
    Buffer();
    Buffer(Buffer&&) = delete;
    Buffer(const Buffer&);
    ~Buffer();


public:
    static Buffer* get_buffer() noexcept;
    Buffer& operator<<(const char*) noexcept;
    Buffer& operator<<(int) noexcept;

    void hold_buffer() noexcept;
    void release_buffer() noexcept;
    const char* retain_buffer() noexcept;
   
private:
    std::string mStream;
    std::mutex* mLock;
    static Buffer* mBuf_p;
};


#endif // _BUFFER_H_
