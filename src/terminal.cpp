#include "terminal.h"

terminal::command_t valid_vfs(std::vector<std::string>& parts) noexcept;
terminal::command_t valid_ls(std::vector<std::string>& parts) noexcept;
terminal::command_t valid_mkdir(std::vector<std::string>& parts) noexcept;
terminal::command_t valid_cd(std::vector<std::string>& parts) noexcept;
terminal::command_t valid_help(std::vector<std::string>& parts) noexcept;
terminal::command_t valid_clear(std::vector<std::string>& parts) noexcept;

VFS* terminal::m_vfs;

constexpr unsigned int hash(const char *s, int off = 0) {
    return !s[off] ? 5381 : (hash(s, off+1)*33) ^ s[off];
}

terminal::terminal() {
    m_vfs = VFS::get_vfs();
    m_mnted_system = &(*m_vfs).get_mnted_system();
    m_cmds = new std::unordered_map<std::string, input_t>();
    init_cmds();
}

terminal::~terminal() {
    delete m_cmds;
    m_vfs->~VFS();
}

void terminal::run() noexcept {
    input();
}

void terminal::input() noexcept {
    std::string line;
    printf("enter /help for cmd list\n-------------------"
           "--\n");

    while(1) {
        printf("-> ");
        std::getline(std::cin, line);

        if(line.empty())
            continue;

        if(line == "exit")
            return;

        std::vector<std::string> parts = split(line.c_str(), SEPARATOR);
        terminal::command_t command = validate_cmd(parts);

        if(command != terminal::help && command != terminal::vfs && command != terminal::clear)
            if(*m_mnted_system == nullptr && command != terminal::invalid) {
                LOG(Log::WARNING, "There is no mounted system");
                continue;
            }

        switch(command) {
            case terminal::help: {
                print_help();
                break;
            }
            case terminal::vfs: {
                if(parts.size() < 2) {
                    m_vfs->vfs_help();
                    break;
                }
                determine_flag(command, parts);
                break;
            }
            case terminal::clear: {
                clear_scr();
                break;
            }
            case terminal::mkdir: {
                ((FAT32*)*m_mnted_system)->mkdir(parts[1].c_str());
                break;
            }
            case terminal::cd: {
                ((FAT32*)*m_mnted_system)->cd(parts[1].c_str());
                break;
            }
            case terminal::ls: {
                ((FAT32*)*m_mnted_system)->ls();
                break;
            }
            case terminal::invalid: printf("command is not found\n"); break;
            default: break;
        }
    }
}

void terminal::init_cmds() noexcept {
    (*m_cmds)["/help"]  = input_t{terminal::help, "lists commands to enter", {}, &valid_help};
    (*m_cmds)["/vfs"]   = input_t{terminal::vfs, "allows the user to access control of the virtual file system",{flag_t("add", wrap_add_disk), flag_t("rm", wrap_rm_disk), flag_t("mnt", wrap_mnt_disk), flag_t("ls", wrap_ls_disk), flag_t{"umnt", wrap_umnt_disk}},&valid_vfs};
    (*m_cmds)["/ls"]    = input_t{terminal::ls, "display the entries within the current working directory", {}, &valid_ls};
    (*m_cmds)["/mkdir"] = input_t{terminal::mkdir, "create directory within current directory", {}, &valid_mkdir};
    (*m_cmds)["/cd"]    = input_t{terminal::cd, "change directory", {}, &valid_cd};
    (*m_cmds)["/clear"] = input_t{terminal::clear, "clears screen", {}, &valid_clear};
}

std::vector<std::string> terminal::split(const char* line, char sep) noexcept {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string x;

    while ((getline(ss, x, sep))) {
        if (x != "")
            tokens.push_back(x);
    }

    return tokens;
}

terminal::command_t terminal::validate_cmd(std::vector<std::string> &parts) noexcept {
    if(m_cmds->find(parts[0]) == m_cmds->end())
        return terminal::invalid;


    return m_cmds->find(parts[0])->second.valid(parts) == terminal::invalid
    ? terminal::invalid : m_cmds->find(parts[0])->second.cmd_name;
}

void terminal::determine_flag(terminal::command_t cmd, std::vector<std::string>& parts) noexcept {
    for(auto i = m_cmds->begin(); i != m_cmds->end(); i++) {
        if(cmd == i->second.cmd_name) {
            for(int j = 0; j < i->second.flags.size(); j++) {
                if(parts[1] == i->second.flags[j].name)
                    i->second.flags[j].func_ptr(parts);
            }
        }
    }
}

void terminal::print_help() noexcept {
    printf("---------  %s  ---------\n", "Help");
    for(auto i = m_cmds->begin(); i != m_cmds->end(); i++) {
        printf(" -> %s - %s\n", i->first.c_str(), i->second.cmd_desc);
    }
    printf("---------  %s  ---------\n", "End");
}

void terminal::clear_scr() const noexcept {
    printf(CLEAR_SCR);
}

terminal::command_t valid_vfs(std::vector<std::string>& parts) noexcept {
    if(parts.size() > 4)
        return terminal::invalid;

    if(parts.size() == 1)
        return terminal::vfs;

    switch(hash(parts[1].c_str())) {
        case hash("ls"): {
            if(parts.size() > 2)
                return terminal::invalid;
            break;
        }
        case hash("add"): {
            if(parts.size() < 3 || parts.size() > 4)
                return terminal::invalid;
            break;
        }
        case hash("rm"): {
            if(parts.size() != 3)
                return terminal::invalid;
            break;
        }
        case hash("mnt"): {
            if(parts.size() != 3)
                return terminal::invalid;
            break;
        }
        case hash("umnt"): {
            if(parts.size() != 2)
                return  terminal::invalid;
            break;
        }
        default: return terminal::invalid;
    }
    return terminal::vfs;
}

terminal::command_t valid_ls(std::vector<std::string>& parts) noexcept {
    if(parts.size() != 1)
        return terminal::invalid;

    return terminal::ls;
}

terminal::command_t valid_mkdir(std::vector<std::string>& parts) noexcept {
    if(parts.size() != 2)
        return terminal::invalid;

    return terminal::mkdir;
}

terminal::command_t valid_cd(std::vector<std::string>& parts) noexcept {
    if(parts.size() != 2)
        return terminal::invalid;

    return terminal::cd;
}

terminal::command_t valid_help(std::vector<std::string>& parts) noexcept {
    if(parts.size() == 1)
        return terminal::help;
    else terminal::invalid;
}

terminal::command_t valid_clear(std::vector<std::string>& parts) noexcept {
    if(parts.size() != 1) {
        return terminal::invalid;
    } else return terminal::clear;
}

void terminal::wrap_add_disk(std::vector<std::string>& parts) {
    m_vfs->add_disk(parts);
}

void terminal::wrap_umnt_disk(std::vector<std::string>& parts) {
    m_vfs->umnt_disk(parts);
}

void terminal::wrap_mnt_disk(std::vector<std::string>& parts) {
    m_vfs->mnt_disk(parts);
}

void terminal::wrap_rm_disk(std::vector<std::string>& parts) {
    m_vfs->rm_disk(parts);
}

void terminal::wrap_ls_disk(std::vector<std::string>& parts) {
    m_vfs->lst_disks(parts);
}


