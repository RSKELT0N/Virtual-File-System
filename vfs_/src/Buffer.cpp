#include "../include/Buffer.h"

Buffer* Buffer::mBuf_p;

Buffer::Buffer() {
    this->mLock = new std::mutex();
    this->mStream = new std::string();
}

Buffer::~Buffer() {
    delete mLock;
    delete mStream;
    free(mBuf_p);
}

Buffer::Buffer(const Buffer&) {

}

Buffer* Buffer::get_buffer() noexcept {
    if(!mBuf_p)
        mBuf_p = new Buffer();
    
    return mBuf_p;
}

void Buffer::hold_buffer() noexcept {
    if(!mStream->empty()) {
        printf("%s", mStream->c_str());
        mStream->clear();
    }
    mLock->lock();
}

void Buffer::release_buffer() noexcept {
    mLock->unlock();
    mStream->clear();
    delete mStream;
    mStream = new std::string();
}

const char* Buffer::retain_buffer() noexcept {
    return mStream->c_str();
}

Buffer& Buffer::operator<<(const char* str) noexcept {
    mStream->append(str);

    return *(this);
}

Buffer& Buffer::operator<<(uint64_t val) noexcept {
    int amt = lib_::countDigit(val);
    char buffer[amt];

    lib_::itoa_(val, buffer, 10, amt);
    buffer[amt] = '\0';
    mStream->append(buffer);

    return (*this);
}

