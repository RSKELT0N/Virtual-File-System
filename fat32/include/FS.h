#ifndef _FS_H_
#define _FS_H_


#include <string>
#include <stdio.h>
#include <string.h>

class FS {

public:
    FILE* get_file_handlr(const char* file_path) noexcept;
    std::string get_ext_file_buffer(const char* file_path) noexcept;
};

#endif // _FS_H_