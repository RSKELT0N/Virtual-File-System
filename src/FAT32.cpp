#include "FAT32.h"

 IFS::IFS() = default;

FAT32::metadata_t::metadata_t(const char* name, uint32_t& dsk_size, uint32_t& clu_size, uint32_t& clu_n) :
        disk_name(name),
        disk_size(dsk_size),
        cluster_size(clu_size),
        cluster_n(clu_n)        {
}

 FAT32::FAT32() {
    this->DISK_NAME = DEFAULT_DISK;
    init();
}

 FAT32::FAT32(const char *disk_name) {
    this->DISK_NAME = disk_name;
    init();
}

FAT32::~FAT32() {
    delete fat_table;
    delete disk;
    delete root;
}

void FAT32::init() noexcept {
     if(access(DISK_NAME, F_OK) == -1)
         set_up();
 }

 void FAT32::set_up() noexcept {
     define_superblock();
     define_fat_table();
 }

 void FAT32::define_superblock() noexcept {
     //FAT32::metadata_t data = {DISK_NAME, STORAGE_SIZE, CLUSTER_SIZE, CLUSTER_N};


 }

void FAT32::cp(const path &src, const path &dst) noexcept {

}

void FAT32::mkdir(char *dir) const noexcept {

}

void FAT32::cd(const path &pth) const noexcept {

}

void FAT32::rm(char *file) noexcept {

}

void FAT32::rm(char *file, const char *args, ...) noexcept {

}
