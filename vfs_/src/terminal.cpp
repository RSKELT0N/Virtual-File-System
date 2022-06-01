#include "../include/terminal.h"

VFS* terminal::m_vfs;
terminal* terminal::m_terminal;

void remote_interpret_cmd(VFS::system_cmd cmd, std::vector<std::string>& args, char*& payload, uint64_t size) noexcept {
    terminal::get_instance()->translate_remote_cmd(cmd, args);
    terminal::get_instance()->interpret_cmd(cmd, args, payload, size);
}

terminal::terminal() {
    sys_lock = new std::mutex();
    m_mnted_system = &(*m_vfs).get_mnted_system();
    m_syscmds = new std::unordered_map<std::string, cmd_t>();
    m_vfs = VFS::get_vfs();

    cmd_map();
}

terminal::~terminal() {
    delete sys_lock;
    delete m_syscmds;
    m_vfs->~VFS();
    BUFFER.~Buffer();
    delete LOG_INSTANCE;
}

terminal* terminal::get_instance() noexcept {
    if(!m_terminal)
        m_terminal = new terminal();

    return m_terminal;
}

void terminal::run() noexcept {
    BUFFER << LOG_str(Log::INFO, "Terminal: defined, /help for cmd list");
    BUFFER << "-------------------------------------------------\n";
    std::string line;

    while(1) {
        
        BUFFER.print_stream();
        printf("%s", ">> ");
        BUFFER.release_buffer();

        std::getline(std::cin, line);
        printf("%s%s%s", "\x1b[A", "\r", "\33[2K");

        if(strcmp(line.c_str(), "/exit") == 0)
            return;

        if(!line.empty()) {
            if(line[0] != ' ') {
                BUFFER.hold_buffer();
                input(line.c_str());
            }
        }
    }
}

void terminal::input(const char* line) noexcept {
    std::vector<std::string> args = lib_::split(line, SEPARATOR);
    VFS::system_cmd command = validate_cmd(args);

    args.erase(args.begin()); // remove initial to retrieve only args
    interpret_cmd(command, args);
}


VFS::system_cmd terminal::validate_cmd(std::vector<std::string> &parts) noexcept {
    if(m_syscmds->find(parts[0]) == m_syscmds->end())
        return VFS::system_cmd::invalid;

    return (*this.*m_syscmds->find(parts[0])->second.valid)(parts) == VFS::system_cmd::invalid
    ? VFS::system_cmd::invalid : m_syscmds->find(parts[0])->second.cmd_name;
}

void terminal::interpret_cmd(VFS::system_cmd cmd, std::vector<std::string>& args, char*& payload, uint64_t size) noexcept {

    if(cmd == VFS::system_cmd::invalid) {
        BUFFER << "command is invalid, use /help\n";
        return;
    }

    (*this.*m_syscmds->find(VFS::syscmd_str[(int)cmd])->second.funct)(cmd, args, payload, size);
}

void terminal::translate_remote_cmd(VFS::system_cmd& cmd, std::vector<std::string>& args) noexcept {
    if(cmd == VFS::system_cmd::cp) {
        if(strcmp(args[0].c_str(), "imp") == 0) {
            args.erase(args.begin());
            args.erase(args.begin());
            cmd = VFS::system_cmd::touch;
        }
    }
}

void terminal::print_help(VFS::system_cmd cmd, std::vector<std::string> args, char*&, uint64_t size) noexcept {
    BUFFER << "\n--------- Help ---------\n";
    uint32_t j = 0;
    for(auto i = m_vfs->get_sys_cmds()->begin(); i != m_vfs->get_sys_cmds()->end(); i++) {
        BUFFER << VFS::syscmd_str[j] << " -> " << i->desc << "\n";
        j++;
    }

    BUFFER << "\nvfs - must start with '/'\n";
    BUFFER << "---------  End  ---------\n";
}

void terminal::clear_scr(VFS::system_cmd cmd, std::vector<std::string> args, char*& payload, uint64_t size) noexcept {
    printf(CLEAR_SCR);
}

void terminal::map_vfs_funct(VFS::system_cmd cmd, std::vector<std::string> args, char*& payload, uint64_t size) noexcept {
    sys_lock->lock();
    if(m_vfs->is_mnted() && strcmp(m_vfs->get_mnted_system()->fs_type, "rfs") == 0) {
        sys_lock->unlock();
        map_sys_funct(cmd, args, payload, size);
    } else m_vfs->control_vfs(args);
    sys_lock->unlock();
}

void terminal::map_sys_funct(VFS::system_cmd cmd, std::vector<std::string> args, char*& payload, uint64_t size) noexcept {
    if(!m_vfs->is_mnted()) {
        BUFFER << LOG_str(Log::WARNING, "Please mount a system before carrying out a sys call");
        return;
    }
    sys_lock->lock();
    (*m_vfs.*m_vfs->get_mnted_system()->access)(cmd, args, payload, size);
    sys_lock->unlock();
}


VFS::system_cmd terminal::valid_vfs(std::vector<std::string>& parts) noexcept {
    if(parts.size() > 6) return VFS::system_cmd::invalid;
    if(parts.size() == 1) return VFS::system_cmd::invalid;

    switch(lib_::hash(parts[1].c_str())) {
        case lib_::hash("ls"):     if(parts.size() > 2)                       return VFS::system_cmd::invalid; break;
        case lib_::hash("ifs"):    if(parts.size() != 4 && parts.size() != 5) return VFS::system_cmd::invalid; break;
        case lib_::hash("rfs"):    if(parts.size() != 4 && parts.size() != 6) return VFS::system_cmd::invalid; break;
        case lib_::hash("mnt"):    if(parts.size() != 3)                      return VFS::system_cmd::invalid; break;
        case lib_::hash("umnt"):   if(parts.size() != 2)                      return VFS::system_cmd::invalid; break;
        case lib_::hash("server"): if(parts.size() != 2 && parts.size() != 3) return VFS::system_cmd::invalid; break;
        default: return VFS::system_cmd::invalid;
    }
    return VFS::system_cmd::vfs_;
}

void terminal::cmd_map() noexcept {
    (*m_syscmds)["/vfs"]   = cmd_t{VFS::system_cmd::vfs_,  &terminal::valid_vfs,   &terminal::map_vfs_funct};
    (*m_syscmds)["ls"]     = cmd_t{VFS::system_cmd::ls,    &terminal::valid_ls,    &terminal::map_sys_funct};
    (*m_syscmds)["mkdir"]  = cmd_t{VFS::system_cmd::mkdir, &terminal::valid_mkdir, &terminal::map_sys_funct};
    (*m_syscmds)["cd"]     = cmd_t{VFS::system_cmd::cd,    &terminal::valid_cd,    &terminal::map_sys_funct};
    (*m_syscmds)["rm"]     = cmd_t{VFS::system_cmd::rm,    &terminal::valid_rm,    &terminal::map_sys_funct};
    (*m_syscmds)["touch"]  = cmd_t{VFS::system_cmd::touch, &terminal::valid_touch, &terminal::map_sys_funct};
    (*m_syscmds)["mv"]     = cmd_t{VFS::system_cmd::mv,    &terminal::valid_mv,    &terminal::map_sys_funct};
    (*m_syscmds)["cp"]     = cmd_t{VFS::system_cmd::cp,    &terminal::valid_cp,    &terminal::map_sys_funct};
    (*m_syscmds)["cat"]    = cmd_t{VFS::system_cmd::cat,   &terminal::valid_cat,   &terminal::map_sys_funct};
    (*m_syscmds)["/help"]  = cmd_t{VFS::system_cmd::help,  &terminal::valid_help,  &terminal::print_help};
    (*m_syscmds)["/clear"] = cmd_t{VFS::system_cmd::clear, &terminal::valid_clear, &terminal::clear_scr};
}

VFS::system_cmd terminal::valid_cp(std::vector<std::string>& parts) noexcept {
    if(parts.size() == 1)
        return VFS::system_cmd::invalid;

    if(strcmp(parts[1].c_str(), "imp") == 0 || strcmp(parts[1].c_str(), "exp") == 0) {
        if(parts.size() == 4)
            return VFS::system_cmd::cp;
    } else if(parts.size() == 3)
        return VFS::system_cmd::cp;

    return VFS::system_cmd::invalid;
}


VFS::system_cmd terminal::valid_ls(std::vector<std::string>& parts)    noexcept { return (parts.size() != 1)   ? VFS::system_cmd::invalid : VFS::system_cmd::ls;      }
VFS::system_cmd terminal::valid_mkdir(std::vector<std::string>& parts) noexcept { return (parts.size() != 2)   ? VFS::system_cmd::invalid : VFS::system_cmd::mkdir;   }
VFS::system_cmd terminal::valid_cd(std::vector<std::string>& parts)    noexcept { return (parts.size() != 2)   ? VFS::system_cmd::invalid : VFS::system_cmd::cd;      }
VFS::system_cmd terminal::valid_rm(std::vector<std::string>& parts)    noexcept { return (parts.size() < 2)    ? VFS::system_cmd::invalid : VFS::system_cmd::rm;      }
VFS::system_cmd terminal::valid_touch(std::vector<std::string>& parts) noexcept { return (parts.size() >  1)   ? VFS::system_cmd::touch   : VFS::system_cmd::invalid; }
VFS::system_cmd terminal::valid_mv(std::vector<std::string>& parts)    noexcept { return (parts.size() != 3)   ? VFS::system_cmd::invalid : VFS::system_cmd::mv;      }
VFS::system_cmd terminal::valid_cat(std::vector<std::string>& parts)   noexcept { return (parts.size() == 2)   ? VFS::system_cmd::cat     : VFS::system_cmd::invalid; }
VFS::system_cmd terminal::valid_help(std::vector<std::string>& parts)  noexcept { return (parts.size() == 1)   ? VFS::system_cmd::help    : VFS::system_cmd::invalid; }
VFS::system_cmd terminal::valid_clear(std::vector<std::string>& parts) noexcept { return (parts.size() == 1)   ? VFS::system_cmd::clear   : VFS::system_cmd::invalid; }
