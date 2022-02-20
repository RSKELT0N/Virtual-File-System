#include "../include/VFS.h"

int itoa_(int value, char *sp, int radix, int amt) {
    char tmp[amt];// be careful with the length of the buffer
    char *tp = tmp;
    uint64_t i = 0;
    unsigned v = 0;

    int sign = (radix == 10 && value < 0);    
    if (sign)
        v = -value;
    else
        v = (unsigned)value;

    while (v || tp == tmp) {
        i = v % radix;
        v /= radix;
        if (i < 10)
          *tp++ = i+'0';
        else
          *tp++ = i + 'a' - 10;
    }

    uint64_t len = tp - tmp;

    if (sign) {
        *sp++ = '-';
        len++;
    }

    while (tp > tmp)
        *sp++ = *--tp;

    return len;
}

constexpr unsigned int hash(const char *s, int off = 0) {
    return !s[off] ? 5381 : (hash(s, off+1)*33) ^ s[off];
}

VFS::system_t::~system_t() {
    if(fs != nullptr) {
        switch(hash(fs_type)) {
                case hash("fat32"): delete static_cast<FAT32*>(fs); break;
                case hash("rfs"):   delete static_cast<Client*>(fs); break;
        }
        printf("Deleted IFS\n");
    }
}

/////////////////////////   VFS   ////////////////////////////

VFS* VFS::vfs;

VFS::VFS() {
    mnted_system = static_cast<system_t*>(malloc(sizeof(system_t)));
    mnted_system->name = "";
    disks        = new std::unordered_map<std::string, system_t>();
    sys_cmds     = new std::vector<VFS::cmd_t>();

    init_sys_cmds();
    BUFFER << LOG_str(Log::INFO, "VFS: defined");
}

VFS::~VFS() {
    if(server != nullptr)
        delete (Server*)server;

    delete mnted_system;
    free(disks);
    free(vfs);
    printf("Deleted VFS\n");
}

const bool VFS::is_mnted() const noexcept {
    return this->mnted_system->fs == nullptr ? false : true;
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
        mnted_system->fs = nullptr;
        mnted_system->fs_type = "";
        mnted_system->name    = "";
    } else BUFFER << LOG_str(Log::WARNING, "There is no system currently mounted");
}

void VFS::mnt_disk(std::vector<std::string>& parts) {
    if(disks->find(parts[1]) == disks->end()) {
        BUFFER << LOG_str(Log::WARNING, "Disk does not exist within the VFS");

        return;
    }

    if(mnted_system->fs != nullptr) {
        BUFFER << LOG_str(Log::WARNING, "Unmount the current system before mounting another");
        return;
    }

    printf("%s  %s  %s", "\n--------------------", parts[1].c_str(), "--------------------\n");
    BUFFER << LOG_str(Log::INFO, "Mounting '" + parts[1] + "' as primary FS on the vfs");

    disks->find(parts[1])->second.fs = typetofs(parts[1].c_str(), disks->find(parts[1])->second.fs_type);

    this->mnted_system->name    = parts[1].c_str();
    this->mnted_system->fs_type = disks->find(parts[1])->second.fs_type;
    this->mnted_system->fs      = *&(disks->find(parts[1])->second.fs);

    if(this->mnted_system->fs_type == "rfs")
        this->mnted_system->access = &VFS::rfs_cmd_func;
    else this->mnted_system->access  = &VFS::ifs_cmd_func;
}

void VFS::add_disk(std::vector<std::string>& parts) {
    if(disks->find(parts[2]) != disks->end()) {
        BUFFER << LOG_str(Log::WARNING, " There is an existing disk with that name already");
        return;
    }

    if(parts.size() == 4) { // added a fourth parameter to specify file system type
        if(fs_types.find(parts[3].c_str()) == fs_types.end()) {
            BUFFER << LOG_str(Log::WARNING, "File system type does not exist");
            return;
        } else (*disks).insert(std::make_pair(parts[2], system_t{parts[2].c_str(), nullptr, parts[3].c_str(), nullptr, {}})); // default fs
    } else (*disks).insert(std::make_pair(parts[2], system_t{parts[2].c_str(), nullptr, DEFAULT_FS, nullptr, {}})); // specified fs
}

void VFS::rm_disk(std::vector<std::string>& parts) {
    if(mnted_system->fs)
        if(disks->find(parts[2])->second.fs == mnted_system->fs)
            umnt_disk(parts); // erases mounted system information, if disk deleted is mounted.
        

    disks->erase(parts[2]); // deletes file system, on heap.
}

void VFS::add_remote(std::vector<std::string>& parts) {

    if(disks->find(parts[2]) != disks->end()) {
        BUFFER << LOG_str(Log::WARNING, "There is an existing remote connection with that name already");
        return;
    }

    (*disks).insert(std::make_pair(parts[2], system_t{parts[2].c_str(), nullptr, "rfs", nullptr, system_t::sock_conn_t{parts[3].c_str(), atoi(parts[4].c_str())}})); // add rfs system with ip and address
}

void VFS::rm_remote(std::vector<std::string>& parts) {
    if(mnted_system->fs)
        if(disks->find(parts[2])->second.fs == mnted_system->fs)
            umnt_disk(parts); // erases mounted system information, if rfs deleted is mounted.

    disks->erase(parts[2]); // deletes file system, on heap.
}

void VFS::lst_disks(std::vector<std::string>& parts) {
    BUFFER << "\n";
    BUFFER << "-----------------  VFS  ---------------\n";
    if(disks->empty()) {
        BUFFER << " -> there is no systems added\n";
        goto no_disks;
    }

    for(auto i = disks->begin(); i != disks->end(); i++) {
        if(i->second.fs_type == "rfs") {
            BUFFER << " -> (name)" << i->first.c_str() << " : (address)" << i->second.conn.addr << ", (port)" << i->second.conn.port;
            goto mount;
        }
        BUFFER << " -> (name)" << i->first.c_str() << " : (filesystem)" << i->second.fs_type;

        mount:
        if(strcmp(mnted_system->name, i->first.c_str()) == 0)
            BUFFER << " %s", "[ Mounted ]";

        BUFFER << "\n";
    }
    
    BUFFER << "----------------  END  ----------------\n\n";
    return;
    
    no_disks:
    BUFFER << "---------------------------------------\n";
}

void VFS::init_server(std::vector<std::string>& args) {
    if(args.size() > 1) {
        if(server != nullptr && strcmp(args[1].c_str(), "logs") == 0)
            BUFFER << (((Server*)((RFS*)server))->print_logs());
        else BUFFER << "Server is not initialised\n";
        return;
    }

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

void VFS::control_vfs(const std::vector<std::string>& parts) noexcept { // checks vfs flags, if none print help. if not, carry out function.
    for(auto & sys_cmd : *sys_cmds) {
        if(VFS::system_cmd::vfs_ == sys_cmd.cmd) {
            for(int j = 0; j < sys_cmd.flags.size(); j++) {
                if(parts[0] == sys_cmd.flags.at(j).name) {
                    std::vector<std::string> tmp(parts);
                    (*this.*sys_cmd.flags.at(j).func)(tmp);
                    return;
                }
            }
        }
    }
    vfs_help();
}


void VFS::ifs_cmd_func(VFS::system_cmd cmd, std::vector<std::string>& args, const char* buffer) noexcept {
    switch(cmd) {
        case VFS::system_cmd::mkdir: (static_cast<IFS*>(VFS::get_vfs()->get_mnted_system()->fs)->mkdir(args[0].c_str())); break;
        case VFS::system_cmd::cd:    (static_cast<IFS*>(VFS::get_vfs()->get_mnted_system()->fs)->cd(args[0].c_str()));    break;
        case VFS::system_cmd::ls:    (static_cast<IFS*>(VFS::get_vfs()->get_mnted_system()->fs)->ls());                   break;
        case VFS::system_cmd::rm:    (static_cast<IFS*>(VFS::get_vfs()->get_mnted_system()->fs)->rm(args));               break;
        case VFS::system_cmd::touch: (static_cast<IFS*>(VFS::get_vfs()->get_mnted_system()->fs)->touch(args, buffer));            break;
        case VFS::system_cmd::mv:    (static_cast<IFS*>(VFS::get_vfs()->get_mnted_system()->fs)->mv(args));               break;
        case VFS::system_cmd::cat:   (static_cast<IFS*>(VFS::get_vfs()->get_mnted_system()->fs)->cat(args[0].c_str()));   break;

        case VFS::system_cmd::cp:    if(args[0] == "imp") 
                                        (static_cast<IFS*>(VFS::get_vfs()->get_mnted_system()->fs)->cp_imp(args[1].c_str(), args[2].c_str()));
                                    else (static_cast<IFS*>(VFS::get_vfs()->get_mnted_system()->fs)->cp(args[0].c_str(), args[1].c_str())); break;
        default: break;
    }
}

void VFS::rfs_cmd_func(VFS::system_cmd cmd, std::vector<std::string>& args, const char* buffer_) noexcept {
    ((Client*)(RFS*)VFS::get_vfs()->get_mnted_system()->fs)->handle_send(syscmd_str[(uint8_t)cmd], (uint8_t)cmd, args);
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
    BUFFER << "------  VFS HELP  ------\n";
    for(int i = 0; i < sys_cmds->at(0).flags.size(); i++) {
        BUFFER << " -> " << sys_cmds->at(0).flags.at(i).name << " - " << sys_cmds->at(0).flags.at(i).desc << "\n";
    }
    BUFFER << "------  END  ------\n";
}

void VFS::load_disks() noexcept {
   std::string path = "disks/";
   std::vector<std::string> func = {"ifs", "add", ""};

   struct dirent *entry;
   DIR *dir = opendir(path.c_str());

   while((entry = readdir(dir)) != NULL) {
       if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
           continue;


       func[2] = entry->d_name;
       VFS::get_vfs()->control_vfs(func);
   }

	closedir(dir);
}

FS* VFS::typetofs(const char* name, const char *fs_type) noexcept {
    switch(hash(fs_type)) {
        case hash("fat32"): return new FAT32(name);
        case hash("rfs"): 
              auto rm = disks->find(name); 
              return new Client(rm->second.conn.addr, rm->second.conn.port); 
    }

    return new FAT32(name);
}
