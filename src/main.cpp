#include "FAT32.h"

int main() {
    IFS* fat32 = new FAT32("file122.dat");
    delete fat32;
    return 0;
}