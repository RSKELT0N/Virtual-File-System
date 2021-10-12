#ifndef _DISK_DRIVER_H_
#define _DISK_DRIVER_H_

#include "Log.h"

class DiskDriver {

protected:
    typedef enum : uint16_t {
        ERROR = 0xFFFF,
        VALID = 0X0000
    } __attribute__((packed))ret_t;

private:
    DiskDriver(const DiskDriver&) = delete;
    DiskDriver(const DiskDriver&&) = delete;

public:
    DiskDriver();
    ~DiskDriver();

    __attribute__((unused)) virtual ret_t close() = 0;
    __attribute__((unused)) virtual ret_t seek(const long& offset) = 0;
    __attribute__((unused)) virtual ret_t truncate(const off_t& size) = 0;
    __attribute__((unused)) virtual ret_t open(const char* pathname, const char* mode) = 0;
    __attribute__((unused)) virtual ret_t read_disk(void* ptr, const size_t& size, const uint32_t& amt) = 0;
    __attribute__((unused)) virtual ret_t write_disk(const void* ptr, const size_t& size, const uint32_t& amt) = 0;
};

#endif //_DISK_DRIVER_H_