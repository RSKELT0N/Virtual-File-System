#include "../include/terminal.h"

VFS* terminal::m_vfs;

constexpr unsigned int hash(const char *s, int off = 0) {
    return !s[off] ? 5381 : (hash(s, off+1)*33) ^ s[off];
}

terminal::terminal() {
    m_vfs = VFS::get_vfs();
    m_mnted_system = &(*m_vfs).get_mnted_system();
    m_extern_cmds = new std::unordered_map<std::string, valid_cmd_t>();

    m_env = terminal::INTERNAL;

    init_valid();
    LOG(Log::INFO, "Terminal: defined, /help for cmd list");
}

terminal::~terminal() {
    LOG(Log::INFO, "Exiting VFS..");
    delete m_extern_cmds;
    std::cout << "Deleted terminal\n";
    m_vfs->~VFS();
}

void terminal::run() noexcept {
    std::string line;

    printf("-------------------------------------------------\n");


    while(1) {
        printf("%s", m_vfs->get_buffr().c_str());
        m_vfs->get_buffr().clear();

        if(m_env == terminal::EXTERNAL)
            printf("%s", "-> ");
        else printf("%s> ", path.c_str());

        std::getline(std::cin, line);

        if(!line.empty())
            input(line.c_str());
    }
}

void terminal::input(const char* line) noexcept {
    uint8_t if_int = interpret_int(line);

    if(if_int)
        return;

    std::vector<std::string> args = m_vfs->split(line, SEPARATOR);
    VFS::system_cmd command = validate_cmd(args);
    terminal::cmd_environment cmd_env = cmdToEnv(command);

    if(cmd_env != m_env && cmd_env != terminal::HYBRID && m_env != terminal::REMOTE) {
        LOG(Log::WARNING, "Command is used within the wrong context");
        return;
    }

    args.erase(args.begin()); // remove initial to retrieve only args
    interpret_ext(command, cmd_env, args);
}

uint8_t terminal::interpret_int(std::string line) noexcept {
    if(m_intern_cmds.find(line) == m_intern_cmds.end())
        return 0;

    switch(hash(line.c_str())) {
        case hash("/clear"): clear_scr(); break;
        case hash("/help"):  print_help(); break;
    }

    return 1;
}

void terminal::interpret_ext(VFS::system_cmd cmd, cmd_environment cmd_env, std::vector<std::string>& args) noexcept {

    if(cmd == VFS::system_cmd::invalid) {
        printf("command is invalid\n");
        return;
    }

    if(cmd_env == HYBRID) {
        switch(cmd) {
            case VFS::system_cmd::vfs_: m_vfs->control_vfs(args);
            determine_env(args[0].c_str());
            break;
        }
    } else if(cmd_env == EXTERNAL) {
            (*m_vfs.*m_vfs->get_mnted_system()->access)(cmd, args, "");
    }
}

void terminal::determine_env(const char *token) noexcept {
    switch(hash(token)) {

        case hash("mnt"):
            if(m_vfs->get_mnted_system() != nullptr)
                set_env(EXTERNAL);
            break;
        case hash("umnt"): 
            if(m_vfs->get_mnted_system() != nullptr)
                set_env(INTERNAL); 
            break;
    }
}

VFS::system_cmd terminal::validate_cmd(std::vector<std::string> &parts) noexcept {
    if(m_extern_cmds->find(parts[0]) == m_extern_cmds->end())
        return VFS::system_cmd::invalid;

    return (*this.*m_extern_cmds->find(parts[0])->second.valid)(parts) == VFS::system_cmd::invalid
    ? VFS::system_cmd::invalid : m_extern_cmds->find(parts[0])->second.cmd_name;
}

void terminal::set_env(cmd_environment env) noexcept {
    this->m_env = env;
}

void terminal::print_help() noexcept {
    printf("---------  %s  ---------\n", "Help");
    for(auto i = m_vfs->get_sys_cmds()->begin(); i != m_vfs->get_sys_cmds()->end(); i++) {
        printf(" -> %s\n", i->desc);
    }
    printf("---------  %s  ---------\n", "End");
}


void terminal::clear_scr() const noexcept {
    printf(CLEAR_SCR);
}

VFS::system_cmd terminal::valid_vfs(std::vector<std::string>& parts) noexcept {
    if(parts.size() > 6)
        return VFS::system_cmd::invalid;

    if(parts.size() == 1)
        return VFS::system_cmd::vfs_;

    switch(hash(parts[1].c_str())) {
        case hash("ls"):
            if(parts.size() > 2)
                return VFS::system_cmd::invalid;
            break;

        case hash("ifs"):
            if(parts.size() != 4 && parts.size() != 5)
                return VFS::system_cmd::invalid;
            break;

        case hash("rfs"):
            if(parts.size() != 4 && parts.size() != 6)
                return VFS::system_cmd::invalid;
            break;

        case hash("mnt"):
            if(parts.size() != 3)
                return VFS::system_cmd::invalid;
            break;

        case hash("umnt"):
            if(parts.size() != 2)
                return VFS::system_cmd::invalid;
            break;

        case hash("server"):
            if(parts.size() != 2 && parts.size() != 3)
                return VFS::system_cmd::invalid;
            break;

        default: return VFS::system_cmd::invalid;
    }
    return VFS::system_cmd::vfs_;
}

VFS::system_cmd terminal::valid_ls(std::vector<std::string>& parts) noexcept {
    if(parts.size() != 1)
        return VFS::system_cmd::invalid;

    return VFS::system_cmd::ls;
}

VFS::system_cmd terminal::valid_mkdir(std::vector<std::string>& parts) noexcept {
    if(parts.size() != 2)
        return VFS::system_cmd::invalid;

    return VFS::system_cmd::mkdir;
}

VFS::system_cmd terminal::valid_cd(std::vector<std::string>& parts) noexcept {
    if(parts.size() != 2)
        return VFS::system_cmd::invalid;

    return VFS::system_cmd::cd;
}

VFS::system_cmd terminal::valid_rm(std::vector<std::string>& parts) noexcept {
    if(parts.size() < 2)
        return VFS::system_cmd::invalid;

    return VFS::system_cmd::rm;
}

VFS::system_cmd terminal::valid_touch(std::vector<std::string>& parts) noexcept {
    if(parts.size() == 2)
        return VFS::system_cmd::touch;
    else return VFS::system_cmd::invalid;
}

VFS::system_cmd terminal::valid_mv(std::vector<std::string>& parts) noexcept {
    if(parts.size() != 3)
        return VFS::system_cmd::invalid;
    else return VFS::system_cmd::mv;
}

VFS::system_cmd terminal::valid_cp(std::vector<std::string>& parts) noexcept {
    if(parts.size() == 3 || parts.size() == 4)
        return VFS::system_cmd::cp;
    else return VFS::system_cmd::invalid;
}

VFS::system_cmd terminal::valid_cat(std::vector<std::string>& parts) noexcept {
    if(parts.size() == 2)
        return VFS::system_cmd::cat;
    else return VFS::system_cmd::invalid;
}

terminal::cmd_environment terminal::cmdToEnv(VFS::system_cmd cmd) noexcept {
    switch(cmd) {
        case VFS::system_cmd::vfs_:  return terminal::HYBRID;
        case VFS::system_cmd::mkdir: return terminal::EXTERNAL;
        case VFS::system_cmd::ls:    return terminal::EXTERNAL;
        case VFS::system_cmd::touch: return terminal::EXTERNAL;
        case VFS::system_cmd::cd:    return terminal::EXTERNAL;
        case VFS::system_cmd::rm:    return terminal::EXTERNAL;
        case VFS::system_cmd::cp:    return terminal::EXTERNAL;
        case VFS::system_cmd::mv:    return terminal::EXTERNAL;
        case VFS::system_cmd::invalid:  return terminal::EXTERNAL;

        default: return m_env;
    }
}

void terminal::init_valid() noexcept {
    (*m_extern_cmds)["/vfs"]   = valid_cmd_t{VFS::system_cmd::vfs_, &terminal::valid_vfs};
    (*m_extern_cmds)["ls"]     = valid_cmd_t{VFS::system_cmd::ls, &terminal::valid_ls};
    (*m_extern_cmds)["mkdir"]  = valid_cmd_t{VFS::system_cmd::mkdir, &terminal::valid_mkdir};
    (*m_extern_cmds)["cd"]     = valid_cmd_t{VFS::system_cmd::cd, &terminal::valid_cd};
    (*m_extern_cmds)["rm"]     = valid_cmd_t{VFS::system_cmd::rm, &terminal::valid_rm};
    (*m_extern_cmds)["touch"]  = valid_cmd_t{VFS::system_cmd::touch, &terminal::valid_touch};
    (*m_extern_cmds)["mv"]     = valid_cmd_t{VFS::system_cmd::mv, &terminal::valid_mv};
    (*m_extern_cmds)["cp"]     = valid_cmd_t{VFS::system_cmd::cp, &terminal::valid_cp};
    (*m_extern_cmds)["cat"]    = valid_cmd_t{VFS::system_cmd::cat, &terminal::valid_cat};
}


