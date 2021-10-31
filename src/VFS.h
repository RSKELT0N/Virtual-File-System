#ifndef _VFS_H_
#define _VFS_H_

#include "IFS.h"
#include "Log.h"
#include <string>
#include <stdio.h>
#include <vector>
#include <unordered_map>

struct VFS {
public:

     typedef struct command_t {
        const char* flag;
        const char* desc;
        command_t(const char* flg, const char* dsc) : flag(flg), desc(dsc) {};
    };

private:
    VFS();
    ~VFS();

    VFS(const VFS&) = delete;
    VFS(VFS&&) = delete;

public:
    void mnt_disk(const std::string& dsk) noexcept;
    void add_disk(const std::string& dsk, IFS* fs) noexcept;
    void rm_disk(const std::string& dsk) noexcept;
    void lst_disks() const noexcept;

public:
    static VFS* get_vfs();
    IFS& get_mnted_system() const noexcept;
    void vfs_help() const noexcept;
    void init_cmds() noexcept;

private:
    static VFS* vfs;
    IFS* mnted_system;
    std::unordered_map<std::string, IFS*>* disks;
    std::vector<VFS::command_t>* vfs_cmds;
};

#endif //_VFS_H_