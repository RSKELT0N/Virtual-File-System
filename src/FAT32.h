#ifndef _FAT32_H_
#define _FAT32_H_

#include "VFS.h"
#include "Disk.h"
#include <string>
#include <vector>
#include <string.h>

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
        uint32_t superblock_size;
        uint32_t fat_table_size;
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

    struct entry_ret_t {
        dir_t* m_dir;
        dir_entry_t* m_entry;

        entry_ret_t(dir_t* dir, dir_entry_t* entry) : m_dir(dir), m_entry(entry) {};
    };

public:
    explicit FAT32(const char* disk_name);
    ~FAT32();

    FAT32(const FAT32& tmp) = delete;
    FAT32(FAT32&& tmp) = delete;

public:
    void cd(const char* pth)  noexcept override;
     void mkdir(const char* dir) noexcept override;
     void rm(std::vector<std::string>& tokens) noexcept override;
     void rm(const char *file, const char* args, ...) noexcept override;
     void cp(const char* src, const char* dst) noexcept override;
     void ls() noexcept override;

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
    void load_superblock() noexcept;
    void load_fat_table() noexcept;

    file_ret store_file(const char* path) noexcept;

    void insert_dir(dir_t& curr_dir, const char* dir_name) noexcept;
    void insert_file(dir_t& dir, const char* path, const char* name) noexcept;
    void delete_entry(entry_ret_t& entry) noexcept;

    dir_t* read_dir(const uint32_t& start_clu) noexcept;
    std::string read_file(dir_t& dir, const char* entry_name) noexcept;
    std::string read_file(dir_entry_t& entry) noexcept;

    uint32_t attain_clu() const noexcept;
    uint32_t n_free_clusters(const uint32_t& req) const noexcept;
    std::vector<uint32_t> get_list_of_clu(const uint32_t& start_clu) noexcept;
    void rm_entr_mem(dir_t& dir, const char* name) noexcept;

    void print_fat_table() const noexcept;
    void print_dir(dir_t& dir) const noexcept;
    void print_super_block() const noexcept;

    FILE* get_file_handlr(const char* file_path) noexcept;

    dir_entry_t* find_entry(dir_t& dir, const char* path, uint8_t shd_exst) const noexcept;
    entry_ret_t* parsePath(std::vector<std::string>& paths, uint8_t shd_exst) noexcept;

private:
    const char* DISK_NAME;
    const char* PATH_TO_DISK;
    static constexpr const char* DEFAULT_DISK = "disk.dat";

    static constexpr uint32_t USER_SPACE    = KB(5);
    static constexpr uint32_t CLUSTER_SIZE  = B(200);
    static constexpr uint32_t CLUSTER_AMT   = USER_SPACE / CLUSTER_SIZE;

    static constexpr size_t   STORAGE_SIZE          = (sizeof(superblock_t) + (sizeof(uint32_t) * CLUSTER_AMT)) + USER_SPACE;
    static constexpr uint32_t SUPERBLOCK_START_ADDR = 0x00000000;
    static constexpr uint32_t FAT_TABLE_START_ADDR  = sizeof(superblock_t);
    static constexpr uint32_t FAT_TABLE_SIZE        = sizeof(uint32_t) * CLUSTER_AMT;
    static constexpr uint32_t ROOT_START_ADDR       = FAT_TABLE_START_ADDR + FAT_TABLE_SIZE;
    static constexpr uint32_t SUPERBLOCK_SIZE       = sizeof(superblock_t);

private:
    Disk* m_disk;
    superblock_t m_superblock;
    uint32_t* m_fat_table;
    dir_t* m_root;
    dir_t* m_curr_dir;
};

#endif //_FAT32_H_