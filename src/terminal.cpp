#include "terminal.h"

terminal::command_t valid_vfs(std::vector<std::string>& parts) noexcept;

constexpr unsigned int hash(const char *s, int off = 0) {
    return !s[off] ? 5381 : (hash(s, off+1)*33) ^ s[off];
}

terminal::terminal() {
    m_vfs = VFS::get_vfs();
    m_mnted_system = (IFS*)&m_vfs->get_mnted_system();
    m_cmds = new std::unordered_map<std::string, input_t>();
}

terminal::~terminal() {

}

void terminal::init_cmds() noexcept {
   (*m_cmds)["/vfs"] = input_t{terminal::vfs, "allows the user to access control of the virtual file system", {"ls", "add", "rm", "mnt"}, &valid_vfs};
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

bool terminal::validate_cmd(std::vector<std::string> &parts) noexcept {

}

terminal::command_t valid_vfs(std::vector<std::string>& parts) noexcept {
    if(parts.size() > 3)
        return terminal::invalid;


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
        default: break;
    }
    return terminal::vfs;
}


