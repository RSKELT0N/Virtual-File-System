#include "../include/Buffer.h"

Buffer* Buffer::mBuf_p;

Buffer::Buffer() {
    this->mStream = new std::string();
    this->mLock = new std::mutex();
}

Buffer::~Buffer() {
    delete mBuf_p;
    delete mLock;
    delete mStream;
}

Buffer* Buffer::get_buffer() noexcept {
    if(!mBuf_p)
        mBuf_p = new Buffer();
    
    return mBuf_p;
}

void Buffer::hold_buffer() const noexcept {
    mLock->lock();
}

const char* Buffer::release_buffer() noexcept {
    mLock->unlock();
    std::string ret = mStream->c_str();
    mStream->clear();
    return ret.c_str();
}

void Buffer::append_buffer(const char* str) noexcept {
    *mStream += std::string(str);
}
