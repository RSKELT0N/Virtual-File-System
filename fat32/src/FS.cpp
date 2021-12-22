#include "../include/FS.h"

FILE* FS::get_file_handlr(const char* file) noexcept {
    FILE* handlr = fopen(file, "r");
    return handlr;
}

std::string FS::get_ext_file_buffer(const char* path) noexcept {
    FILE* file = get_file_handlr(path);
    fseek(file, 0, SEEK_END);
    size_t fsize = ftell(file);
    rewind(file);

    char buffer[fsize];
    fread(buffer, sizeof(char), fsize, file);

    return std::string(buffer);
}