#ifndef _IFS_H_
#define _IFS_H_

#include "Path.h"
#include <string>

class IFS {
public:
    IFS() = delete;
    IFS(const IFS&) = delete;
    IFS(IFS&&) = delete;

protected:
    virtual void cd(const path& pth) const noexcept = 0;
    virtual void mkdir(char* dir) const noexcept = 0;
    virtual void rm(char* file) noexcept = 0;
    virtual void rm(char *file, const char* args, ...) noexcept = 0;
    virtual void cp(const path& src, const path& dst) noexcept = 0;

};

#endif //_IFS_H_