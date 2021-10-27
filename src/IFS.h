#ifndef _IFS_H_
#define _IFS_H_


class IFS {
public:
    IFS();
    IFS(const IFS&) = delete;
    IFS(IFS&&) = delete;

protected:
    virtual void cd(const char* pth) const noexcept = 0;
    virtual void mkdir(char* dir) const noexcept = 0;
    virtual void rm(char* file) noexcept = 0;
    virtual void rm(char *file, const char* args, ...) noexcept = 0;
    virtual void cp(const char* src, const char* dst) noexcept = 0;
};

#endif //_IFS_H_