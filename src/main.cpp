#include <stdio.h>
#include <iostream>

int main() {
    FILE* out = fopen("test.txt", "wb+");
    std::cout << out << "\n";
    fclose(out);
    std::cout << out << "\n";
    return 0;
}