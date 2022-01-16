#include "../include/terminal.h"

int main(int argc, char** argv) {
    VFS::get_vfs()->load_disks();
    terminal* term = new terminal();
    term->run();

    delete term;
    return 0;
}