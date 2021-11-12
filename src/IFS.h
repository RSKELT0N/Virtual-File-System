#ifndef _IFS_H_
#define _IFS_H_

#include <vector>
#include <string>

class IFS {
public:
    IFS();
    IFS(const IFS&) = delete;
    IFS(IFS&&) = delete;

protected:
    virtual void cd(const char* pth) noexcept = 0;
    virtual void mkdir(const char* dir) noexcept = 0;
    virtual void rm(std::vector<std::string>& tokens) noexcept = 0;
    virtual void rm(const char *file, const char* args, ...) noexcept = 0;
    virtual void cp(const char* src, const char* dst) noexcept = 0;
    virtual void ls() noexcept = 0;
};

#endif //_IFS_H_