#include "../include/Buffer.h"

Buffer* Buffer::mBuf_p;

Buffer::Buffer() {
    this->mLock = new std::mutex();
    this->mStream = new std::vector<char>();
}

Buffer::~Buffer() {
    delete mLock;
    delete mStream;
    free(mBuf_p);
}

Buffer* Buffer::get_buffer() noexcept {
    if(!mBuf_p)
        mBuf_p = new Buffer();
    
    return mBuf_p;
}

void Buffer::hold_buffer() noexcept {
     if(!mStream->empty()) {
         printf("\r");
         print_stream();
         mStream->clear();
     }
    mLock->lock();
}

void Buffer::release_buffer() noexcept {
    mLock->unlock();
    mStream->clear();
    delete mStream;
    mStream = new std::vector<char>();
}

void Buffer::retain_buffer(char*& store) noexcept {
    store = (char*)malloc(sizeof(char) * mStream->size());
    std::copy(mStream->begin(), mStream->end(), store);
}

void Buffer::retain_buffer(std::byte*& store) noexcept {
    store = (std::byte*)malloc(sizeof(std::byte) * (mStream->size()));
    std::copy(mStream->begin(), mStream->end(), (char*)&(*store));
}


Buffer& Buffer::operator<<(const char* str) noexcept {
    for(int i = 0; i < strlen(str); i++) {
        mStream->push_back(str[i]);
    }

    return *(this);
}

Buffer& Buffer::operator<<(uint64_t val) noexcept {
    int amt = lib_::countDigit(val);
    char buffer[amt];

    lib_::itoa_(val, buffer, 10, amt);
    buffer[amt] = '\0';

    *this << buffer;

    return (*this);
}

void Buffer::append(char* str, size_t len) noexcept {
    for(int i = 0; i < len; i++) {
        mStream->push_back(str[i]);
    }
}

void Buffer::print_stream() noexcept {
    for(int i = 0; i < mStream->size(); i++) {
        printf("%c", (*mStream)[i]);
    }
}

