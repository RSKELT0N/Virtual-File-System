#include "terminal.h"

int main() {
    terminal* term = new terminal();
    term->run();
    delete term;

//    int* a = new int(10);
//    int** ptr = &a;
//    delete a;#
//    a = new int(15);
//
//    printf("%d", **ptr);
    return 0;
}