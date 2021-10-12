#ifndef _VFS_H_
#define _VFS_H_

#include "IFS.h"
#include <string>
#include <unordered_map>

struct VFS {

private:
    VFS();
    VFS(const VFS&) = delete;
    VFS(VFS&&) = delete;

public:
    ~VFS();

public:
    void mnt_disk(const std::string& dsk) noexcept;
    void add_disk(const std::string& dsk, IFS* fs) noexcept;
    void rm_disk(const std::string& dsk) noexcept;

    static VFS* get_vfs();

private:
    static VFS* vfs;
    std::unordered_map<std::string, VFS>* disks;
};

#endif //_VFS_H_