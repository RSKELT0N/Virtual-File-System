#include "../include/Buffer.h"

int countDigit(int n)
{
    int count = 0;
    while (n != 0)
    {
        n = n / 10;
        ++count;
    }
    return count;
}

extern int itoa_(int value, char *sp, int radix, int amt);

Buffer* Buffer::mBuf_p;

Buffer::Buffer() {
    this->mLock = new std::mutex();
}

Buffer::~Buffer() {
    delete mBuf_p;
    delete mLock;
}

Buffer::Buffer(const Buffer&) {

}

Buffer* Buffer::get_buffer() noexcept {
    if(!mBuf_p)
        mBuf_p = new Buffer();
    
    return mBuf_p;
}

void Buffer::hold_buffer() noexcept {
    mLock->lock();
}

void Buffer::release_buffer() noexcept {
    mLock->unlock();
    mStream.clear();
}

const char* Buffer::retain_buffer() noexcept {
    return mStream.c_str();
}

Buffer& Buffer::operator<<(const char* str) noexcept {
    mStream += str;

    const char* tmp = mStream.c_str();

    return *(this);
}

Buffer& Buffer::operator<<(int val) noexcept {
    int amt = countDigit(val);
    char buffer[amt];

    itoa_(val, buffer, 10, amt);
    buffer[amt] = '\0';
    mStream += buffer;

    return (*this);
}
