#include "../include/FS.h"

std::string FS::convert_size(const uint64_t& bytes_) const noexcept {
    long double ret = 0;
    long double bytes = static_cast<long double>(bytes_);

    char buffer[10];
    memset(buffer, '\0', 10);

    if(bytes > ((long double)(1 << 30))) {
        ret = bytes / (1 << 30);
        sprintf(buffer, "%03.2LfGb", ret);
        goto end;
    }

    if(bytes > ((long double)(1 << 20))) {
        ret = bytes / (1 << 20);
        sprintf(buffer, "%03.2LfMb", ret);
        goto end;
    }

    if(bytes > ((long double)(1 << 10))) {
        ret = bytes / (1 << 10);
        sprintf(buffer, "%03.2LfKb", ret);
        goto end;
    }

    ret = bytes;
    if(buffer[0] == '\0')
        sprintf(buffer, "%03.2LfBy", ret);

    end:

    return std::string(std::string(buffer));
}

FILE* FS::get_file_handlr(const char* file, char* type) noexcept {
    FILE* handlr = fopen(file, type);
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

void FS::store_ext_file_buffer(const char* path, char*& payload, uint64_t size) noexcept {
    FILE* file = get_file_handlr(path, "w");

    if(file != NULL) {
        return;
    }

    fwrite(payload, size, sizeof(char), file);
    fclose(file);
}