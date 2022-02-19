#include "../include/FS.h"

FILE* FS::get_file_handlr(const char* file) noexcept {
    FILE* handlr = fopen(file, "r");
    return handlr;
}

void FS::get_ext_file_buffer(const char* path, char*& payload) noexcept {
    FILE* file = get_file_handlr(path);

    if(file == NULL) {
        return;
    }

    fseek(file, 0, SEEK_END);
    uint64_t fsize = ftell(file);
    rewind(file);

    payload = (char*)malloc(sizeof(char) * fsize);
    fread(payload, sizeof(char), fsize, file);
    fclose(file);
}