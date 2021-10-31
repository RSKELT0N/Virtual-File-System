#include "VFS.h"

VFS* VFS::vfs;

VFS::VFS() {
    disks = new std::unordered_map<std::string, IFS*>();
    vfs_cmds = new std::vector<VFS::command_t>();
    mnted_system = nullptr;

    init_cmds();
}

VFS::~VFS() {
    delete vfs;
    delete mnted_system;
    delete disks;
    delete vfs_cmds;
}

void VFS::mnt_disk(const std::string &dsk) noexcept {
    IFS* tmp = nullptr;
    for(auto i = disks->begin(); i != disks->end(); i++) {
        if(dsk == i->first)
            tmp = i->second;
    }

    if(tmp == nullptr) {
        LOG(Log::WARNING, "disk does not exist within the VFS");
        return;
    }

    this->mnted_system = tmp;
}

void VFS::add_disk(const std::string &dsk, IFS *fs) noexcept {
    disks->insert({dsk, fs});
}

void VFS::rm_disk(const std::string &dsk) noexcept {
    disks->erase(dsk);
}

void VFS::lst_disks() const noexcept {
    printf("%s\n-------------------\n", " Disks");
    for(auto i = disks->begin(); i != disks->end(); i++) {
        printf(" -> (name)%s : (filesystem)%s\n", i->first.c_str(), i->second->fs_name());
    }
    printf("-------------------\n");
}

VFS* VFS::get_vfs() {
    if(vfs == nullptr)
        vfs = new VFS();
    return vfs;
}

IFS& VFS::get_mnted_system() const noexcept {
    return *mnted_system;
}

void VFS::init_cmds() noexcept {
    command_t ls = {"ls", "lists the curre`t mounted systems"};
    command_t add = {"add", "adds a system to the virtual file system"};
    command_t rm = {"rm", "removes a system to the virtual file system"};

    vfs_cmds->push_back(ls);
    vfs_cmds->push_back(add);
    vfs_cmds->push_back(rm);
}

void VFS::vfs_help() const noexcept {
    printf("------  %s  ------", "VFS help");
}