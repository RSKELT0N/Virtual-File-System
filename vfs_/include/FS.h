#ifndef _FS_H_
#define _FS_H_


#include <string>
#include <stdio.h>
#include <string.h>

class FS {

public:
    std::string convert_size(const uint64_t&) const noexcept;
    FILE* get_file_handlr(const char* file_path) noexcept;
    void get_ext_file_buffer(const char* file_path, char*&) noexcept;
    void store_ext_file_buffer(const char* file_path, char*&, uint64_t size) noexcept;
};

#endif // _FS_H_