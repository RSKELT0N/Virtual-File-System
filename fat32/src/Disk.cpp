#include <cstdio>

#include "../include/Disk.h"

DiskDriver::DiskDriver() = default;
DiskDriver::~DiskDriver() = default;

Disk::Disk() {
    file = nullptr;
}

Disk::~Disk() {
    if(!file) {
        LOG(Log::WARNING, "FILE object has not been initialised yet!");
        exit(EXIT_FAILURE);
    }
    fclose(file);
    std::cout << "Deleted Disk\n";
}

FILE* Disk::get_file() const noexcept {
    if(!file)
        LOG(Log::WARNING, "FILE cannot be returned as it has not been initialised!");

    return file;
}

size_t& Disk::get_addr() const noexcept {
    return (size_t&)addr;
}

DiskDriver::ret_t Disk::open(const char *pathname, const char *mode) {
    this->cmpl_path_to_file = "disks/" + std::string(pathname);
    this->disk_name = pathname;

    this->file = fopen(cmpl_path_to_file.c_str(), mode);

    if(file == NULL)
        LOG(Log::ERROR_, "File descriptor could not be opened.");

    return file == nullptr ? ERROR : VALID;
}

DiskDriver::ret_t Disk::close() {
    if(file == nullptr)
        LOG(Log::WARNING, "FD can't be closed, as it's not initialised");

    uint32_t val = fclose(get_file());

    return val == EOF ? ERROR : VALID;
}

DiskDriver::ret_t Disk::read(void* ptr, const size_t& size, const uint32_t& amt) {
    size_t ttl_amt = fread(ptr, size, amt, file);

    if(ttl_amt != amt)
        LOG(Log::ERROR_, "Error reading disk at '" + std::string(std::to_string(addr)) + "'.");

    return ttl_amt == amt ? VALID : ERROR;
}

DiskDriver::ret_t Disk::write(const void *ptr, const size_t &size, const uint32_t &amt) {
    size_t ttl_amt = fwrite(ptr, size, amt, file);

    if(ttl_amt != amt)
        LOG(Log::ERROR_, "Error writing disk at '" + std::string(std::to_string(addr)) + "'.");

    return ttl_amt == amt ? VALID : ERROR;
}

DiskDriver::ret_t Disk::seek(const long &offset) {
    uint8_t val = fseek(file, offset, SEEK_SET);

    if(val == -1)
        LOG(Log::ERROR_, "Error setting offset address from 'SEEK_SET' within disk.");

    return val == -1 ? ERROR : VALID;
}

DiskDriver::ret_t Disk::truncate(const off_t &size) {
    uint8_t val = ftruncate(fileno(file), size);

    if(val == -1)
        LOG(Log::ERROR_, "Error truncating the file");

    return val == -1 ? ERROR : VALID;
}

DiskDriver::ret_t Disk::rm() {
    uint8_t val = std::remove(cmpl_path_to_file.c_str());
    if(val != 0)
        LOG(Log::WARNING, "Cant remove file(" +  cmpl_path_to_file + ")");

    return val == 0 ? VALID : ERROR;
}
