#include <cstdio>
#include "Disk.h"

DiskDriver::DiskDriver() = default;
DiskDriver::~DiskDriver() = default;

Disk::Disk() {
    file = NULL;
}

Disk::~Disk() {
    if(file == NULL) {
        LOG(Log::WARNING, "FILE object has not been initialised yet!");
        exit(EXIT_FAILURE);
    }
    delete file;
}

FILE* Disk::get_file() const noexcept {
    if(file == nullptr)
        LOG(Log::WARNING, "FILE cannot be returned as it has not been initialised!");

    return file;
}

size_t& Disk::get_addr() const noexcept {
    return (size_t&)addr;
}

DiskDriver::ret_t Disk::open(const char *pathname, const char *mode) {
    this->file = fopen(pathname, mode);

    if(file != NULL)
        LOG(Log::INFO, "File descriptor for '" + std::string(pathname) + "' has been defined/opened.");
    else LOG(Log::ERROR, "File descriptor could not be opened.");

    return file == nullptr ? ERROR : VALID;
}

DiskDriver::ret_t Disk::close() {
    if(file == nullptr)
        LOG(Log::WARNING, "FD can't be closed, as it's not initialised");

    uint32_t val = fclose(get_file());
    LOG(Log::INFO, "File descriptor has been closed");

    return val == EOF ? ERROR : VALID;
}

DiskDriver::ret_t Disk::read(void* ptr, const size_t& size, const uint32_t& amt) {
    size_t ttl_amt = fread(ptr, size, amt, file);

    if(ttl_amt != amt)
        LOG(Log::ERROR, "Error reading disk at '" + std::string(std::to_string(addr)) + "'.");

    return ttl_amt == amt ? VALID : ERROR;
}

DiskDriver::ret_t Disk::write(const void *ptr, const size_t &size, const uint32_t &amt) {
    size_t ttl_amt = fwrite(ptr, size, amt, file);

    if(ttl_amt != amt)
        LOG(Log::ERROR, "Error writing disk at '" + std::string(std::to_string(addr)) + "'.");

    return ttl_amt == amt ? VALID : ERROR;
}

DiskDriver::ret_t Disk::seek(const long &offset) {
    uint8_t val = fseek(file, offset, SEEK_SET);

    if(val == -1)
        LOG(Log::ERROR, "Error setting offset address from 'SEEK_SET' within disk.");

    return val == -1 ? ERROR : VALID;
}

DiskDriver::ret_t Disk::truncate(const off_t &size) {
    uint8_t val = ftruncate(fileno(file), size);

    if(val == -1)
        LOG(Log::ERROR, "Error truncating the file");

    return val == -1 ? ERROR : VALID;
}
