#include "../include/VFS.h"

VFS* VFS::vfs;

constexpr unsigned int hash(const char *s, int off = 0) {
    return !s[off] ? 5381 : (hash(s, off+1)*33) ^ s[off];
}

VFS::system_t::~system_t() {
    if(fs != nullptr) {
        delete (FAT32*)fs;
        fs = nullptr;
    }
}

VFS::VFS() {
    disks = new std::unordered_map<std::string, system_t>();
    mnted_system = (system_t*)malloc(sizeof(system_t));
    mnted_system->fs = nullptr;
    mnted_system->name = "";

    sys_cmds = new std::vector<VFS::cmd_t>();

    init_sys_cmds();
    LOG(Log::INFO, "VFS: defined");
}

VFS::~VFS() {
    if(server != nullptr)
        delete (Server*)server;

    delete mnted_system;
    free(disks);
    std::cout << "Deleted disks\n";

    free(vfs);
    std::cout << "Deleted VFS\n";
}

VFS::system_t*& VFS::get_mnted_system() noexcept {
    return mnted_system;
}

VFS*& VFS::get_vfs() noexcept {
    if(!vfs)
        vfs = new VFS();

    return vfs;
}

std::vector<VFS::cmd_t>* VFS::get_sys_cmds() noexcept {
    return sys_cmds;
}

void VFS::umnt_disk(std::vector<std::string> &parts) {
    if(mnted_system->fs != nullptr) {
        delete (FAT32*)mnted_system->fs;
        mnted_system->fs = nullptr;
        mnted_system->fs_type = "";
        mnted_system->name    = "";
    } else LOG(Log::WARNING, "There is no system currently mounted");
}

void VFS::mnt_disk(std::vector<std::string>& parts) {
    if(disks->find(parts[1]) == disks->end()) {
        LOG(Log::WARNING, "Disk does not exist within the VFS");
        return;
    }

    if(mnted_system->fs != nullptr) {
        LOG(Log::WARNING, "Unmount the current system before mounting another");
        return;
    }

    printf("%s  %s  %s", "\n--------------------", parts[1].c_str(), "--------------------\n");
    LOG(Log::INFO, "Mounting '" + parts[1] + "' as primary FS on the vfs");

    disks->find(parts[1])->second.fs = typetofs(parts[1].c_str(), disks->find(parts[1])->second.fs_type);

    this->mnted_system->name    = parts[1].c_str();
    this->mnted_system->fs_type = parts[1].c_str();
    this->mnted_system->fs      = *&disks->find(parts[1])->second.fs;
    this->mnted_system->access  = &VFS::ifs_cmd_func;
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
        } else (*disks).insert(std::make_pair(parts[2], system_t{parts[2].c_str(), nullptr, parts[3].c_str(), nullptr, {}}));
    } else (*disks).insert(std::make_pair(parts[2], system_t{parts[2].c_str(), nullptr, DEFAULT_FS, nullptr, {}}));
}

void VFS::rm_disk(std::vector<std::string>& parts) {
    if(mnted_system->fs)
        if(disks->find(parts[2])->second.fs == mnted_system->fs) {
            delete (FAT32*)disks->find(parts[2])->second.fs;
            this->mnted_system->fs = nullptr;
        }

    disks->erase(parts[2]);
}

void VFS::add_remote(std::vector<std::string>& parts) {
    if(disks->find(parts[2]) != disks->end()) {
        LOG(Log::WARNING, "There is an existing remote connection with that name already");
        return;
    }

    (*disks).insert(std::make_pair(parts[2], system_t{parts[2].c_str(), nullptr, "rfs", nullptr, system_t::sock_conn_t{parts[3].c_str(), atoi(parts[4].c_str())}}));
}

void VFS::rm_remote(std::vector<std::string>& parts) {
    if(mnted_system->fs)
        if(disks->find(parts[2])->second.fs == mnted_system->fs) {
            delete (Client*)disks->find(parts[2])->second.fs;
            this->mnted_system->fs = nullptr;
        }

    disks->erase(parts[2]);
}

void VFS::lst_disks(std::vector<std::string>& parts) {
    printf("\n");
    if(disks->empty()) {
        printf("-----------------  %s  ---------------\n", "VFS");
        printf(" %s", "-> there is no systems added\n");
        goto no_disks;
    }

    printf("--------------  %s  --------------\n", "Systems");
    for(auto i = disks->begin(); i != disks->end(); i++) {
        if(i->second.fs_type == "rfs") {
            printf(" -> (name)%s : (address)%s, (port)%d", i->first.c_str(), i->second.conn.addr, i->second.conn.port);
            goto mount;
        }
        printf(" -> (name)%s : (filesystem)%s", i->first.c_str(), i->second.fs_type);

        mount:
        if(strcmp(mnted_system->name, i->first.c_str()) == 0)
            printf(" %s", "[ Mounted ]");
        printf("\n");
    }
    
    printf("----------------  %s  ----------------\n\n", "END");
    return;
    
    no_disks:
    printf("---------------------------------------\n");
}

void VFS::init_server(std::vector<std::string>&) {
    if(server == nullptr) {
        server = new Server();
        return;
    }

    delete (Server*)server;
    server = nullptr;
}

void VFS::init_sys_cmds() noexcept {
    sys_cmds->push_back({system_cmd::vfs_,
                         {flag_t{"ls",     &VFS::lst_disks,   "lists the current mounted systems                          | -> [/vfs ls]"},
                          flag_t{"ifs",    &VFS::control_ifs, "controls internal file systems within the vfs             | -> [/vfs ifs add/rm <DISK_NAME> <FS_TYPE>]"},
                          flag_t{"rfs",    &VFS::control_rfs, "controls remote file systems within the vfs               | -> [/vfs rfs add/rm <NAME> <IP> <PORT>]"},
                          flag_t{"mnt",    &VFS::mnt_disk,    "initialises the file system and mounts it towards the vfs | -> [/vfs mnt <DISK_NAME>]"},
                          flag_t{"umnt",   &VFS::umnt_disk,   "deletes file system data/disk from vfs                   | -> [/vfs umnt"},
                          flag_t{"server", &VFS::init_server, "toggles server initialisation for client connection on local host on specified port"}},
                         "allows the user to access control of the virtual file system"});


    sys_cmds->push_back({system_cmd::ls,     {}, "display the entries within the current working directory"});
    sys_cmds->push_back({system_cmd::mkdir,  {}, "create directory within current directory"});
    sys_cmds->push_back({system_cmd::cd,     {}, "changes the current directory within the file system"});
    sys_cmds->push_back({system_cmd::rm,     {}, "removes an entry within the file system"});
    sys_cmds->push_back({system_cmd::touch,  {}, "creates an entry within the file system"});
    sys_cmds->push_back({system_cmd::mv,     {}, "moves an entry towards a different directory"});
    sys_cmds->push_back({system_cmd::cp,     {}, "copies an entry within the specified directory"});
    sys_cmds->push_back({system_cmd::cat,    {}, "print bytes found at entry"});
}

void VFS::control_vfs(std::vector<std::string>& parts) noexcept {
    for(auto & sys_cmd : *sys_cmds) {
        if(VFS::system_cmd::vfs_ == sys_cmd.cmd) {
            for(int j = 0; j < sys_cmd.flags.size(); j++) {
                if(parts[0] == sys_cmd.flags.at(j).name) {
                    (*this.*sys_cmd.flags.at(j).func)(parts);
                    return;
                }
            }
        }
    }
    vfs_help();
}

void VFS::control_ifs(std::vector<std::string>& parts) noexcept {
    switch(hash(parts[1].c_str())) {
        case hash("add"): add_disk(parts); return;
        case hash("rm"):  rm_disk(parts);  return;
    }
}

void VFS::control_rfs(std::vector<std::string>& parts) noexcept {
    switch(hash(parts[1].c_str())) {
        case hash("add"): add_remote(parts); return;
        case hash("rm"):  rm_remote(parts);  return;
    }
}

void VFS::vfs_help() const noexcept {
    printf("------  %s  ------\n", "VFS help");
    for(int i = 0; i < sys_cmds->at(0).flags.size(); i++) {
        printf(" -> %s - %s\n", sys_cmds->at(0).flags.at(i).name, sys_cmds->at(0).flags.at(i).desc);
    }
    printf("------  %s  ------\n", "END");
}

std::vector<std::string> VFS::split(const char* line, char sep) noexcept {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string x;

    while ((getline(ss, x, sep))) {
        if (x != "")
            tokens.push_back(x);
    }

    return tokens;
}

FS* VFS::typetofs(const char* name, const char *fs_type) noexcept {
    switch(hash(fs_type)) {
        case hash("fat32"): {
            return (FAT32*)new FAT32(name);
        }
        case hash("rfs"): {
            auto rm = disks->find(name);
            return (Client*)new Client(rm->second.conn.addr, rm->second.conn.port); 
        }
    }
    return (FAT32*)new FAT32(name);
}

void VFS::ifs_cmd_func(VFS::system_cmd cmd, std::vector<std::string>& args) noexcept {
    switch(cmd) {
        case VFS::system_cmd::mkdir: {
            ((IFS*)VFS::get_vfs()->get_mnted_system()->fs)->mkdir(args[0].c_str());
            break;
        }
        case VFS::system_cmd::cd: {
            ((IFS*)VFS::get_vfs()->get_mnted_system()->fs)->cd(args[0].c_str());
            break;
        }
        case VFS::system_cmd::ls: {
            ((IFS*)VFS::get_vfs()->get_mnted_system()->fs)->ls();
            break;
        }
        case VFS::system_cmd::rm: {
            ((IFS*)VFS::get_vfs()->get_mnted_system()->fs)->rm(args);
            break;
        }
        case VFS::system_cmd::touch: {
            ((IFS*)VFS::get_vfs()->get_mnted_system()->fs)->touch(args);
            break;
        }
        case VFS::system_cmd::mv: {
            ((IFS*)VFS::get_vfs()->get_mnted_system()->fs)->mv(args);
            break;
        }
        case VFS::system_cmd::cat: {
            ((IFS*)VFS::get_vfs()->get_mnted_system()->fs)->cat(args[0].c_str());
            break;
        }
        case VFS::system_cmd::cp: {
            if(args[0] == "ext")
                ((IFS*)VFS::get_vfs()->get_mnted_system()->fs)->cp_ext(args[1].c_str(), args[2].c_str());
            else ((IFS*)VFS::get_vfs()->get_mnted_system()->fs)->cp(args[0].c_str(), args[1].c_str());
            break;
        }
        default: break;
    }
}

std::stringstream& VFS::get_buffr() noexcept {
    return buffr;
}

void VFS::append_buffr(const char* txt) noexcept {
    buffr << txt;
}