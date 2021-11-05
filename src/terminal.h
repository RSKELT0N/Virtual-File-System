#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include "VFS.h"
#include "FAT32.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>

#define SEPARATOR (char)' '
#define CLEAR_SCR (const char*)"\033c"

class terminal {
public:
    enum command_t {
        help,
        vfs,
        ls,
        mkdir,
        cd,
        touch,
        cp,
        mv,
        clear,
        invalid
    };

    struct flag_t {
        typedef void (*func)(std::vector<std::string>&);

        const char* name;
        func func_ptr;
        flag_t(const char* flg, func ptr) : name(flg), func_ptr(ptr) {};
    };

    typedef struct {
        command_t cmd_name;
        const char* cmd_desc;
        std::vector<flag_t> flags;
        terminal::command_t (*valid)(std::vector<std::string>& parts);
    } input_t;

public:
    terminal();
    ~terminal();

    terminal(const terminal&) = delete;
    terminal(terminal&&) = delete;

    void run() noexcept;

    friend terminal::command_t valid_vfs(std::vector<std::string>& parts) noexcept;
    friend terminal::command_t valid_ls(std::vector<std::string>& parts) noexcept;
    friend terminal::command_t valid_mkdir(std::vector<std::string>& parts) noexcept;
    friend terminal::command_t valid_cd(std::vector<std::string>& parts) noexcept;
    friend terminal::command_t valid_help(std::vector<std::string>& parts) noexcept;
    friend terminal::command_t valid_clear(std::vector<std::string>& parts) noexcept;

private:
    void input() noexcept;
    void init_cmds() noexcept;
    void print_help() noexcept;
    std::vector<std::string> split(const char* line, char sep) noexcept;
    command_t validate_cmd(std::vector<std::string>& parts) noexcept;
    void determine_flag(command_t cmd, std::vector<std::string>&) noexcept;
    void clear_scr() const noexcept;

    static void wrap_add_disk(std::vector<std::string>&);
    static void wrap_umnt_disk(std::vector<std::string>&);
    static void wrap_mnt_disk(std::vector<std::string>&);
    static void wrap_rm_disk(std::vector<std::string>&);
    static void wrap_ls_disk(std::vector<std::string>&);

public:
    static VFS* m_vfs;

private:
    IFS** m_mnted_system;
    std::unordered_map<std::string, input_t>* m_cmds;
};

#endif // _TERMINAL_H_