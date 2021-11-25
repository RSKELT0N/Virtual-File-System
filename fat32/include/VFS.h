#ifndef _VFS_H_
#define _VFS_H_

#include <set>
#include <string>
#include <vector>
#include <stdio.h>
#include <unordered_map>


#include "FS.h"
#include "IFS.h"
#include "Log.h"

#include "FAT32.h"

#define DEFAULT_FS (const char*)"fat32"

struct VFS {

public:
    enum system_cmd {
        vfs_,
        invalid,
        ls,
        mkdir,
        cd,
        rm,
        touch,
        cp,
        mv,
        cat
    };

    struct flag_t {
         const char* name;
         void (VFS::*func)(std::vector<std::string>&) = NULL;
         const char* desc;
         explicit flag_t(const char* nm, void(VFS::*ptr)(std::vector<std::string>&), const char* dsc) : name(nm), func(ptr), desc(dsc) {}
    };

    struct cmd_t {
        system_cmd cmd;
        std::vector<flag_t> flags;
        const char* desc;
    };

    struct system_t {
        const char* name;
        FS* fs = nullptr;
        const char* fs_type;
        void (VFS::*access)(system_cmd cmd, std::vector<std::string>& args);

        ~system_t();
        system_t(const char* nme, FS* _fs, const char* type, void (VFS::*ptr)(VFS::system_cmd, std::vector<std::string>&)): name(nme), fs(_fs), fs_type(type), access(ptr)   {};
    };

private:
    VFS();
    VFS(const VFS&) = delete;
    VFS(VFS&&) = delete;

public:
    ~VFS();
    void init_sys_cmds() noexcept;

    IFS* typetofs(const char* name, const char* fs_type) noexcept;
    void control_vfs(std::vector<std::string>&) noexcept;
    void ifs_cmd_func(VFS::system_cmd cmd, std::vector<std::string>& args) noexcept;

public:
    static VFS*& get_vfs() noexcept;
    system_t*& get_mnted_system() noexcept;
    std::vector<VFS::cmd_t>* get_sys_cmds() noexcept;

public:
     void umnt_disk(std::vector<std::string>&);
     void mnt_disk(std::vector<std::string>&);
     void add_disk(std::vector<std::string>&);
     void rm_disk(std::vector<std::string>&);
     void lst_disks(std::vector<std::string>&);
     void vfs_help() const noexcept;

private:
    static VFS* vfs;
    system_t* mnted_system;
    std::vector<VFS::cmd_t>* sys_cmds;
    std::set<std::string> fs_types = {"fat32"};
    std::unordered_map<std::string, system_t>* disks;

};

#endif //_VFS_H_