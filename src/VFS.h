#ifndef _VFS_H_
#define _VFS_H_

#include "IFS.h"
#include "Log.h"
#include "FAT32.h"
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
     void mnt_disk(std::vector<std::string>&);
     void add_disk(std::vector<std::string>&);
     void rm_disk(std::vector<std::string>&);
     void lst_disks(std::vector<std::string>&);

public:
    static VFS* get_vfs();
    IFS* get_mnted_system() const noexcept;
    void vfs_help() const noexcept;
    void init_cmds() noexcept;

private:
    static VFS* vfs;
    IFS* mnted_system;
    std::unordered_map<std::string, IFS*>* disks;
    std::vector<VFS::command_t>* vfs_cmds;
};

#endif //_VFS_H_