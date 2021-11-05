#ifndef _VFS_H_
#define _VFS_H_

#include "IFS.h"
#include "Log.h"
#include "FAT32.h"
#include <set>
#include <string>
#include <stdio.h>
#include <vector>
#include <unordered_map>

#define DEFAULT_FS (const char*)"fat32"

struct VFS {
    ~VFS();

public:

     typedef struct command_t {
        const char* flag;
        const char* desc;
        command_t(const char* flg, const char* dsc) : flag(flg), desc(dsc) {};
    };

     typedef struct system_t {
         const char* fs_type;
         IFS* fs;
         system_t(const char* type, IFS* sys) : fs_type(type), fs(sys) {};
     };

private:
    VFS();

    VFS(const VFS&) = delete;
    VFS(VFS&&) = delete;

public:
     void umnt_disk(std::vector<std::string>&);
     void mnt_disk(std::vector<std::string>&);
     void add_disk(std::vector<std::string>&);
     void rm_disk(std::vector<std::string>&);
     void lst_disks(std::vector<std::string>&);

public:
    static VFS* get_vfs();
    IFS*& get_mnted_system() noexcept;
    void vfs_help() const noexcept;
    void init_cmds() noexcept;

private:
    IFS* typetofs(const char* name, const char* fs_type) noexcept;

private:
    static VFS* vfs;
    IFS* mnted_system;
    std::unordered_map<std::string, system_t>* disks;
    std::vector<VFS::command_t>* vfs_cmds;
    std::set<std::string> fs_types = {"fat32"};
};

#endif //_VFS_H_