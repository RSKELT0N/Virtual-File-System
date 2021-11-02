#include "terminal.h"

terminal::command_t valid_vfs(std::vector<std::string>& parts) noexcept;
VFS* terminal::m_vfs;

constexpr unsigned int hash(const char *s, int off = 0) {
    return !s[off] ? 5381 : (hash(s, off+1)*33) ^ s[off];
}

terminal::terminal() {
    m_vfs = VFS::get_vfs();
    m_mnted_system = (IFS**)m_vfs->get_mnted_system();
    m_cmds = new std::unordered_map<std::string, input_t>();
    init_cmds();
}

terminal::~terminal() {
    delete m_cmds;
}

void terminal::run() noexcept {
    input();
}

void terminal::input() noexcept {
    std::string line;

    while(1) {
        std::getline(std::cin, line);

        if(line.empty())
            continue;

        if(line == "exit")
            return;

        std::vector<std::string> parts = split(line.c_str(), SEPARATOR);
        terminal::command_t command = validate_cmd(parts);

        switch(command) {
            case terminal::vfs: {
                if(parts.size() < 2) {
                    m_vfs->vfs_help();
                    break;
                }
                determine_flag(command, parts);
                break;
            }
            case terminal::invalid: printf("command is not found\n"); break;
            default: break;
        }
    }
}

void terminal::init_cmds() noexcept {
   (*m_cmds)["/vfs"] = input_t{terminal::vfs, "allows the user to access control of the virtual file system", {flag_t("add", wrap_add_disk), flag_t("rm", wrap_rm_disk), flag_t("mnt", wrap_mnt_disk), flag_t("ls", wrap_ls_disk)}, &valid_vfs};
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

terminal::command_t valid_vfs(std::vector<std::string>& parts) noexcept {
    if(parts.size() > 3)
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
            if(parts.size() != 3)
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
        default: return terminal::invalid;
    }
    return terminal::vfs;
}

void terminal::wrap_add_disk(std::vector<std::string>& parts) {
    m_vfs->add_disk(parts);
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

