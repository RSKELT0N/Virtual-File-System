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

void VFS::mnt_disk(std::vector<std::string>& parts) {
    IFS* tmp = nullptr;
    for(auto i = disks->begin(); i != disks->end(); i++) {
        if(parts[2] == i->first)
            tmp = i->second;
    }

    if(tmp == nullptr) {
        LOG(Log::WARNING, "disk does not exist within the VFS");
        return;
    }

    this->mnted_system = tmp;
}

void VFS::add_disk(std::vector<std::string>& parts) {
    disks->insert({parts[2], (IFS*)new FAT32(parts[2].c_str())});
}

void VFS::rm_disk(std::vector<std::string>& parts) {
    disks->erase(parts[2]);
}

void VFS::lst_disks(std::vector<std::string>& parts) {
    printf("%s\n----------------------------------------\n", " Disks");
    if(disks->empty()) {
        printf(" %s", "-> there is no active systems added\n");
        goto no_disks;
    }
    for(auto i = disks->begin(); i != disks->end(); i++) {
        printf(" -> (name)%s : (filesystem)%s", i->first.c_str(), i->second->fs_name());
        if(i->second == mnted_system)
            printf(" %s", "[ Mounted ]");
        printf("\n");
    }
    no_disks:
    printf("-----------------------------------------\n");
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
    command_t ls = {"ls", "lists the current mounted systems"};
    command_t add = {"add", "adds a system to the virtual file system"};
    command_t rm = {"rm", "removes a system to the virtual file system"};

    vfs_cmds->push_back(ls);
    vfs_cmds->push_back(add);
    vfs_cmds->push_back(rm);
}

void VFS::vfs_help() const noexcept {
    printf("------  %s  ------\n", "VFS help");
    for(int i = 0; i < vfs_cmds->size(); i++) {
        printf(" -> %s - %s\n", vfs_cmds->at(i).flag, vfs_cmds->at(i).desc);
    }
    printf("------  %s  ------\n", "END");
}