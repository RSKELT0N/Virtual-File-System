#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "VFS.h"

#define SEPARATOR (char)' '
#define CLEAR_SCR (const char*)"\033c"

class terminal {
    
public:
    enum cmd_environment {
        EXTERNAL,
        HYBRID,
        INTERNAL
    };

    typedef struct {
        VFS::system_cmd cmd_name = {};
        VFS::system_cmd (terminal::*valid)(std::vector<std::string>& parts);
    } valid_cmd_t;

public:
    terminal();
    ~terminal();
    terminal(const terminal&) = delete;
    terminal(terminal&&) = delete;

public:
    void run() noexcept;

private:
    void input(const char* line) noexcept;
    uint8_t interpret_int(std::string line) noexcept;
    void interpret_ext(VFS::system_cmd cmd, cmd_environment cmd_env, std::vector<std::string>& args) noexcept;
    void determine_env(const char* token) noexcept;

    void init_valid() noexcept;
    void set_env(cmd_environment env) noexcept;
    VFS::system_cmd validate_cmd(std::vector<std::string>& parts) noexcept;
    terminal::cmd_environment cmdToEnv(VFS::system_cmd cmd) noexcept;

    void print_help() noexcept;
    void clear_scr() const noexcept;

public:
    VFS::system_cmd valid_vfs(std::vector<std::string>& parts) noexcept;
    VFS::system_cmd valid_ls(std::vector<std::string>& parts) noexcept;
    VFS::system_cmd valid_mkdir(std::vector<std::string>& parts) noexcept;
    VFS::system_cmd valid_cd(std::vector<std::string>& parts) noexcept;
    VFS::system_cmd valid_rm(std::vector<std::string>& parts) noexcept;
    VFS::system_cmd valid_touch(std::vector<std::string>& parts) noexcept;
    VFS::system_cmd valid_mv(std::vector<std::string>& parts) noexcept;
    VFS::system_cmd valid_cp(std::vector<std::string>& parts) noexcept;
    VFS::system_cmd valid_cat(std::vector<std::string>& parts) noexcept;

public:
    static VFS* m_vfs;

private:
    std::string path;
    cmd_environment m_env;
    VFS::system_t** m_mnted_system;
    std::unordered_set<std::string> m_intern_cmds = {"/clear", "/help"};
    std::unordered_map<std::string, valid_cmd_t>* m_extern_cmds;
};

#endif // _TERMINAL_H_