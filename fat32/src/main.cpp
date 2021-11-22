#include "../include/terminal.h"

int main() {
    terminal* term = new terminal();
    term->run();
    delete term;

    return 0;
}
