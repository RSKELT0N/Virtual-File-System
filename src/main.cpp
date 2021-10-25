#include "FAT32.h"

//With 1 entry in one cluster, error seems to occur with reading directory

int main() {
    IFS* fat32 = new FAT32("disk.dat");
    delete fat32;
    return 0;
}