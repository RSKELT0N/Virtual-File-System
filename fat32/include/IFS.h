#ifndef _IFS_H_
#define _IFS_H_

#include <vector>
#include <string>

#include "config.h"
#include "FS.h"

class IFS : public FS {
public:
    IFS();
    IFS(const IFS&) = delete;
    IFS(IFS&&) = delete;

public:
    __attribute__((unused)) virtual void cd(const char* pth) noexcept = 0;
    __attribute__((unused)) virtual void mkdir(const char* dir) noexcept = 0;
    __attribute__((unused)) virtual void rm(std::vector<std::string>& tokens) noexcept = 0;
    __attribute__((unused)) virtual void mv(std::vector<std::string>& tokens) noexcept = 0;
    __attribute__((unused)) virtual void cp(const char* src, const char* dst) noexcept = 0;
    __attribute__((unused)) virtual void cp_ext(const char* src, const char* dst) noexcept = 0;
    __attribute__((unused)) virtual void touch(std::vector<std::string>& tokens, const char* buffer) noexcept = 0;
    __attribute__((unused)) virtual void cat(const char* path) noexcept = 0;
    __attribute__((unused)) virtual void ls() noexcept = 0;
};

#endif //_IFS_H_