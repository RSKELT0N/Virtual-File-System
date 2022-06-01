#include "../include/FS.h"

std::string FS::convert_size(const uint64_t& bytes_) const noexcept {
    long double ret = 0;
    long double bytes = static_cast<long double>(bytes_);

    char buffer[10];
    memset(buffer, '\0', 10);

    if(bytes > ((long double)(1 << 30))) {
        ret = bytes / (1 << 30);
        sprintf(buffer, "%03.2LfGB", ret);
        goto end;
    }

    if(bytes > ((long double)(1 << 20))) {
        ret = bytes / (1 << 20);
        sprintf(buffer, "%03.2LfMB", ret);
        goto end;
    }

    if(bytes > ((long double)(1 << 10))) {
        ret = bytes / (1 << 10);
        sprintf(buffer, "%03.2LfKB", ret);
        goto end;
    }

    ret = bytes;
    if(buffer[0] == '\0')
        sprintf(buffer, "%03.2LfB", ret);

    end:

    return std::string(std::string(buffer));
}

FILE* FS::get_file_handlr(const char* file, char* type) noexcept {
    FILE* handlr = fopen(file, type);
    return handlr;
}

uint64_t FS::get_ext_file_buffer(const char* path, std::byte*& payload) noexcept {
    FILE* file = get_file_handlr(path);

    if(file == NULL) {
        return 0;
    }

    fseek(file, 0, SEEK_END);
    uint64_t fsize = ftell(file);
    rewind(file);


    payload = new std::byte[fsize];
    fread(payload, sizeof(std::byte), fsize, file);
    fclose(file);
    return fsize;
}

long FS::get_file_size(const char* path) noexcept {
    FILE* file = get_file_handlr(path);
    fseek(file, 0L, SEEK_END);
    long sz = ftell(file);
    fclose(file);
    return sz;
}

void FS::store_ext_file_buffer(const char* path, std::byte* payload, uint64_t size) noexcept {
    FILE* file = get_file_handlr(path, (char*)"w+");

    ftruncate(fileno(file), size);
    rewind(file);
    fclose(file);

    FILE* file_ = get_file_handlr(path, (char*)"r+");
    fwrite(payload, get_file_size(path), sizeof(char), file);
    fclose(file);
}