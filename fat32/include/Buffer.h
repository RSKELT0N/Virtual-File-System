#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <mutex>
#include <string.h>

class Buffer {
   
private:
    Buffer();
    ~Buffer();
    Buffer(Buffer&&) = delete;
    Buffer(const Buffer&) = delete;

public:
    static Buffer* get_buffer() noexcept;
    void append_buffer(const char*) noexcept;

    void hold_buffer() const noexcept;
    const char* release_buffer() noexcept;
   
private:
    std::string* mStream;
    std::mutex* mLock;
    static Buffer* mBuf_p;
};


#endif // _BUFFER_H_
