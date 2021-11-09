#include "VFS.h"

constexpr unsigned int hash(const char *s, int off = 0) {
    return !s[off] ? 5381 : (hash(s, off+1)*33) ^ s[off];
}

VFS::system_t::~system_t() {
    if(this->fs)
        delete (FAT32*)fs;
}

VFS::VFS() {
    disks = new std::unordered_map<std::string, system_t>();
    vfs_cmds = new std::vector<VFS::command_t>();
    mnted_system = (system_t*)malloc(sizeof(system_t));
    mnted_system->fs_type = "";
    mnted_system->fs = nullptr;

    init_cmds();
}

VFS::~VFS() {
    if(!mnted_system)
        delete mnted_system;
    delete disks;
    std::cout << "Deleted disks\n";
    delete vfs_cmds;
    std::cout << "Deleted VFS\n";
}

void VFS::umnt_disk(std::vector<std::string> &parts) {
    if(mnted_system->fs != nullptr) {
        delete (FAT32*)mnted_system->fs;
        mnted_system->fs = nullptr;
        mnted_system->fs_type = "";
    } else LOG(Log::WARNING, "There is no system currently mounted");
}

void VFS::mnt_disk(std::vector<std::string>& parts) {
    if(disks->find(parts[2]) == disks->end()) {
        LOG(Log::WARNING, "Disk does not exist within the VFS");
        return;
    }

    if(mnted_system->fs != nullptr) {
        LOG(Log::WARNING, "Unmount the current system before mounting another");
        return;
    }

    disks->find(parts[2])->second.fs = typetofs(parts[2].c_str(), disks->find(parts[2])->second.fs_type);
    this->mnted_system->fs_type = parts[2].c_str();
    this->mnted_system->fs = (FAT32*)disks->find(parts[2])->second.fs;
}

void VFS::add_disk(std::vector<std::string>& parts) {
    if(disks->find(parts[2]) != disks->end()) {
        LOG(Log::WARNING, " There is an existing disk with that name already");
        return;
    }

    if(parts.size() == 4) {
        if(fs_types.find(parts[3].c_str()) == fs_types.end()) {
            LOG(Log::WARNING, "File system type does not exist");
            return;
        } else (*disks).insert(std::make_pair(parts[2], system_t{parts[3].c_str()}));
    } else (*disks).insert(std::make_pair(parts[2], system_t{DEFAULT_FS}));
}

void VFS::rm_disk(std::vector<std::string>& parts) {
    if(mnted_system)
        if(disks->find(parts[2])->second.fs == mnted_system->fs) {
            delete (FAT32*)disks->find(parts[2])->second.fs;
            this->mnted_system->fs = nullptr;
        }

    disks->erase(parts[2]);
}

void VFS::lst_disks(std::vector<std::string>& parts) {
    printf("%s\n----------------------------------------\n", " Disks");
    if(disks->empty()) {
        printf(" %s", "-> there is no active systems added\n");
        goto no_disks;
    }
    for(auto i = disks->begin(); i != disks->end(); i++) {
        printf(" -> (name)%s : (filesystem)%s", i->first.c_str(), i->second.fs_type);
        if(!(strcmp(i->first.c_str(), mnted_system->fs_type) != 0))
            printf(" %s", "[ Mounted ]");
        printf("\n");
    }
    no_disks:
    printf("-----------------------------------------\n");
}

VFS::system_t*& VFS::get_mnted_system() noexcept {
    printf("vfs: %p\n", &mnted_system);
    return mnted_system;
}

void VFS::init_cmds() noexcept {
    command_t ls = {"ls", "lists the current mounted systems                          | -> [/vfs ls]"};
    command_t add = {"add", "adds a system to the virtual file system                  | -> [/vfs add <DISK_NAME> <FS_TYPE>]"};
    command_t rm = {"rm", "removes a system to the virtual file system                | -> [/vfs rm <DISK_NAME>]"};
    command_t mnt = {"mnt", "initialises the file system and mounts it towards the vfs | -> [/vfs mnt <DISK_NAME>]"};
    command_t umnt = {"umnt", "deletes file system data/disk from vfs                   | -> [/vfs umnt <DISK_NAME>]"};

    vfs_cmds->push_back(ls);
    vfs_cmds->push_back(add);
    vfs_cmds->push_back(rm);
    vfs_cmds->push_back(mnt);
    vfs_cmds->push_back(umnt);
}

void VFS::vfs_help() const noexcept {
    printf("------  %s  ------\n", "VFS help");
    for(int i = 0; i < vfs_cmds->size(); i++) {
        printf(" -> %s - %s\n", vfs_cmds->at(i).flag, vfs_cmds->at(i).desc);
    }
    printf("------  %s  ------\n", "END");
}

IFS* VFS::typetofs(const char* name, const char *fs_type) noexcept {
    switch(hash(fs_type)) {
        case hash("fat32"): {
            return (FAT32*)new FAT32(name);
        }
    }
}