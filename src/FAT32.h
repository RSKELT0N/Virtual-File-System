#ifndef _FAT32_H_
#define _FAT32_H_

#include "VFS.h"
#include "Disk.h"
#include <string.h>
#include <vector>

#define B(__size__)               (__size__)
#define KB(__size__)              (B(__size__) * 1024)
#define MB(__size__)              (KB(__size__) * 1024)
#define GB(__size__)              (MB(__size__) * 1024)
#define CLUSTER_ADDR(__CLUSTER__) (KB(2) * __CLUSTER__)

#define min(a,b) ((a) < (b) ? (a) : (b))
#define DISK_NAME_LENGTH (uint8_t)10
#define DIR_NAME_LENGTH  (uint8_t)10
#define UNDEF_START_CLUSTER 0
#define DIRECTORY 1
#define NON_DIRECTORY 0


class FAT32 : public IFS {

public:
     enum clu_values_t {
        UNALLOCATED_CLUSTER = 0x00000000,
        BAD_CLUSTER         = 0x00000FF7,
        EOF_CLUSTER         = 0x00000FF8,
        ALLOCATED_CLUSTER   = 0x00000001
    };

     struct file_ret {
        uint32_t start_cluster;
        FILE* fd;

        file_ret(uint32_t clu, FILE* fd) : start_cluster(clu), fd(fd) {};
    };

private:

    struct metadata_t {
        char disk_name[DISK_NAME_LENGTH];
        uint32_t disk_size;
        uint32_t user_size;
        uint32_t cluster_size;
        uint32_t cluster_n;

//        metadata_t(const char* name, uint32_t& dsk_size, uint32_t& clu_size, uint32_t& clu_n);
    } __attribute__((packed));

    typedef struct __attribute__((packed)) {
        metadata_t data;
        uint32_t superblock_addr;
        uint32_t fat_table_addr;
        uint32_t root_dir_addr;
    } superblock_t;

    typedef struct __attribute__((packed)) {
        char dir_name[DIR_NAME_LENGTH];
        uint32_t dir_entry_amt;
        uint32_t start_cluster_index;
        uint32_t parent_cluster_index;
    } dir_header_t;

    typedef struct __attribute__((packed)) {
        char dir_entry_name[DIR_NAME_LENGTH];
        uint32_t start_cluster_index;

        uint32_t dir_entry_size;
        uint8_t is_directory;
    } dir_entry_t;

    typedef struct __attribute__((packed)) {
        dir_header_t dir_header;
        dir_entry_t* dir_entries;
    } dir_t;

public:
    explicit FAT32(const char* disk_name);
    ~FAT32();

    FAT32(const FAT32& tmp) = delete;
    FAT32(FAT32&& tmp) = delete;

public:
    void cd(const char* pth) const noexcept override;
     void mkdir(char* dir) const noexcept override;
     void rm(char* file) noexcept override;
     void rm(char *file, const char* args, ...) noexcept override;
     void cp(const char* src, const char* dst) noexcept override;

private:
    void init() noexcept;
    void set_up() noexcept;
    void create_disk() noexcept;
    void load() noexcept;
    void add_new_entry(dir_t& dir, const char* name, const uint32_t& start_clu, const uint32_t& size, const uint8_t& is_dir) noexcept;

    void define_superblock() noexcept;
    void define_fat_table() noexcept;
    dir_t* init_dir(const uint32_t& start_cl, const uint32_t& parent_clu, const char* name) noexcept;

    void store_superblock() noexcept;
    void store_fat_table() noexcept;
    void store_dir(dir_t& directory) noexcept;
    file_ret store_file(const char* path) noexcept;

    void insert_dir(dir_t& curr_dir, const char* dir_name) noexcept;
    void insert_file(dir_t& dir, const char* path) noexcept;

    dir_t* read_dir(const uint32_t& start_clu) noexcept;

    uint32_t attain_clu() const noexcept;
    uint32_t n_free_clusters(const uint32_t& req) const noexcept;
    std::vector<uint32_t> get_list_of_clu(const uint32_t& start_clu) noexcept;

    void print_fat_table() const noexcept;
    void print_dir(dir_t& dir) const noexcept;

    FILE* get_file_handlr(const char* file_path) noexcept;

private:
    const char* DISK_NAME;
    static constexpr const char* DEFAULT_DISK = "disk.dat";

    static constexpr uint32_t USER_SPACE    = KB(6);
    static constexpr uint32_t CLUSTER_SIZE  = B(200);
    static constexpr uint32_t CLUSTER_AMT   = USER_SPACE / CLUSTER_SIZE;

    static constexpr size_t   STORAGE_SIZE          = (sizeof(superblock_t) + (sizeof(uint32_t) * CLUSTER_AMT)) + USER_SPACE;
    static constexpr uint32_t SUPERBLOCK_START_ADDR = 0x00000000;
    static constexpr uint32_t FAT_TABLE_START_ADDR  = sizeof(superblock_t);
    static constexpr uint32_t FAT_TABLE_SIZE        = sizeof(uint32_t) * CLUSTER_AMT;
    static constexpr uint32_t ROOT_START_ADDR       = FAT_TABLE_START_ADDR + FAT_TABLE_SIZE;

private:
    Disk* m_disk;
    superblock_t m_superblock;
    uint32_t* m_fat_table;
    dir_t* m_root;
};

#endif //_FAT32_H_