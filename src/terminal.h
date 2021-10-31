#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include "VFS.h"
#include "FAT32.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>

#define SEPARATOR (char)' '

class terminal {
public:
    enum command_t {
        vfs,
        ls,
        mkdir,
        touch,
        cp,
        mv,
        invalid
    };

    typedef struct {
        command_t cmd_name;
        const char* cmd_desc;
        std::unordered_set<const char*> flags;
        terminal::command_t (*valid)(std::vector<std::string>& parts);
    } input_t;

public:
    friend terminal::command_t valid_vfs(std::vector<std::string>& parts) noexcept;

public:
    terminal();
    ~terminal();

    terminal(const terminal&) = delete;
    terminal(terminal&&) = delete;

private:
    std::vector<std::string> split(const char* line, char sep) noexcept;

public:
    void init_cmds() noexcept;
    bool validate_cmd(std::vector<std::string>& parts) noexcept;

public:
    VFS* m_vfs;

private:
    IFS* m_mnted_system;
    std::unordered_map<std::string, input_t>* m_cmds;
};

#endif // _TERMINAL_H_