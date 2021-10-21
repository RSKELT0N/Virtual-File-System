#include "FAT32.h"

 IFS::IFS() = default;

 FAT32::FAT32(const char *disk_name) {
     if(disk_name[0] == '\0')
         DISK_NAME = DEFAULT_DISK;
     else DISK_NAME = disk_name;

     m_disk      = new Disk();
     m_fat_table = (uint8_t*)malloc(sizeof(uint8_t) * CLUSTER_AMT);
     m_root      = (dir_t*)malloc(sizeof(dir_t));

    init();
}

FAT32::~FAT32() {
    free(m_fat_table);
    delete m_disk;
    delete m_root;
}

void FAT32::init() noexcept {
     if(access(DISK_NAME, F_OK) == -1)
         set_up();
     else LOG(Log::WARNING, "'" + std::string(DISK_NAME) + "' disk already exists.");
 }

 void FAT32::set_up() noexcept {
     define_superblock();
     define_fat_table();
     m_root = init_dir(0xFF, 0xFF, "root");

     create_disk();
     store_superblock();
     store_fat_table();
     store_dir(reinterpret_cast<dir_t &>(m_root));
     LOG(Log::INFO, "file system has been initialised.");
 }

 void FAT32::create_disk() noexcept {
     m_disk->open(DISK_NAME, (const char*)"wb");
     m_disk->truncate(STORAGE_SIZE);
     rewind(m_disk->get_file());

     m_disk->close();

     m_disk->open(DISK_NAME, "rb+");
     LOG(Log::INFO, "binary file '" + std::string(DISK_NAME) + "' has been created.");
 }

 void FAT32::define_superblock() noexcept {
     FAT32::metadata_t data{};
     strcpy(data.disk_name, DISK_NAME);
     data.cluster_size = CLUSTER_SIZE;
     data.disk_size    = STORAGE_SIZE;
     data.cluster_n    = CLUSTER_AMT;
     data.user_size    = USER_SPACE;

     m_superblock.data = data;
     m_superblock.superblock_addr = SUPERBLOCK_START_ADDR;
     m_superblock.fat_table_addr  = FAT_TABLE_START_ADDR;
     m_superblock.root_dir_addr   = ROOT_START_ADDR;
 }

 void FAT32::define_fat_table() noexcept {
     memset((void*)m_fat_table, UNALLOCATED_CLUSTER, (size_t)FAT_TABLE_SIZE);
 }

 FAT32::dir_t* FAT32::init_dir(const uint32_t &start_cl, const uint32_t &parent_clu, const char* name) noexcept {
     dir_header_t hdr;
     auto* tmp = (dir_t*)malloc(sizeof(dir_t));
     strcpy(hdr.dir_name, name);
     hdr.start_cluster_index = 0;
     hdr.parent_cluster_index = 0;
     hdr.dir_entry_amt = 2;

     tmp->dir_header = hdr;
     tmp->dir_entries = (dir_entry_t*)malloc(sizeof(dir_entry_t) * tmp->dir_header.dir_entry_amt);

     strcpy(tmp->dir_entries[0].dir_entry_name, ".");
     tmp->dir_entries[0].start_cluster_index = UNDEF_START_CLUSTER;
     tmp->dir_entries[0].is_directory = DIRECTORY;
     tmp->dir_entries[0].dir_entry_size = sizeof(tmp->dir_entries[0]);

     strcpy(tmp->dir_entries[1].dir_entry_name, "..");
     tmp->dir_entries[1].start_cluster_index = UNDEF_START_CLUSTER;
     tmp->dir_entries[1].is_directory = DIRECTORY;
     tmp->dir_entries[1].dir_entry_size = sizeof(tmp->dir_entries[1]);

     return tmp;
 }

 void FAT32::store_superblock() noexcept {
     m_disk->seek(m_superblock.superblock_addr);
     m_disk->write((void*)&m_superblock, sizeof(m_superblock), 1);
     fflush(m_disk->get_file());
 }

 void FAT32::store_fat_table() noexcept {
     m_disk->seek(m_superblock.fat_table_addr);
     m_disk->write(m_fat_table, sizeof(uint8_t), (uint32_t)CLUSTER_AMT);
     fflush(m_disk->get_file());
 }

 void FAT32::store_dir(dir_t& directory)  noexcept {
     // ensuring header data is able to fit within a cluster size
     if(CLUSTER_SIZE < sizeof(directory.dir_header))
         LOG(Log::ERROR, "Insufficient memory to store header data for directory");

     // //////////////////////////////////////////////////////////////////////////////////////////////
     // initialise variables for workout within cluster size

     uint16_t first_clu_entry_amt = (CLUSTER_SIZE - sizeof(directory.dir_header)) / sizeof(dir_entry_t);
     uint16_t remain_entries  = (first_clu_entry_amt >= directory.dir_header.dir_entry_amt) ? 0 : (directory.dir_header.dir_entry_amt - (uint32_t)first_clu_entry_amt);
     uint16_t amt_of_entry_per_clu = 0;

     // //////////////////////////////////////////////////////////////////////////////////////////////
     // get first cluster and store header data within
     if(!n_free_clusters(1)) {
         LOG(Log::ERROR, "amount of clusters needed is not valid");
         return;
     }

     uint32_t first_clu_index = attain_clu();

     directory.dir_header.start_cluster_index = first_clu_index;

     m_disk->seek(m_superblock.root_dir_addr + (first_clu_index * CLUSTER_SIZE));
     m_disk->write((void*)&directory.dir_header, sizeof(dir_header_t), 1);
     fflush(m_disk->get_file());

     if(remain_entries > 0)
         amt_of_entry_per_clu = CLUSTER_SIZE / sizeof(dir_entry_t);

     uint16_t num_of_clu_needed = (uint16_t)((double)remain_entries / (double)amt_of_entry_per_clu);

     // //////////////////////////////////////////////////////////////////////////////////////////////
     // write entries in first cluster index
     for(int i = 0; i < min(directory.dir_header.dir_entry_amt, first_clu_entry_amt); i++) {
         size_t addr_offset = (ROOT_START_ADDR + (first_clu_index * CLUSTER_SIZE) + sizeof(dir_header_t));

         m_disk->seek(addr_offset + (i * sizeof(dir_entry_t)));
         m_disk->write((void*)&directory.dir_entries[i], sizeof(dir_entry_t), 1);
         fflush(m_disk->get_file());
     }
     // /////////////////////////////////////////////////////////////////////////////////////////////
     // add remaining entries within available clusters, up until last cluster

     if(remain_entries == 0) {
         m_fat_table[first_clu_index] = EOF_CLUSTER;
         store_fat_table();
         return;
     }

     if(amt_of_entry_per_clu % num_of_clu_needed > 0)
         num_of_clu_needed++;


     if(!n_free_clusters(num_of_clu_needed)) {
         LOG(Log::WARNING, "remaining entries cannot be stored due to insufficient cluster amount");
         m_fat_table[first_clu_index] = UNALLOCATED_CLUSTER;
         return;
     }

     uint32_t* clu_list = (uint32_t*)malloc(sizeof(uint32_t) * num_of_clu_needed);

     for(int i = 0; i < num_of_clu_needed; i++)
         clu_list[i] = attain_clu();

     uint32_t nxt_clu = first_clu_entry_amt + 1;

     for(int i = 0; i < num_of_clu_needed - 1; i++) {
         size_t addr_offset = (ROOT_START_ADDR + (first_clu_index * CLUSTER_SIZE) + sizeof(dir_header_t));

         m_disk->seek(addr_offset + (nxt_clu * sizeof(dir_entry_t)));
         m_disk->write((void*)&directory.dir_entries[nxt_clu], sizeof(dir_entry_t), amt_of_entry_per_clu);
         fflush(m_disk->get_file());
         nxt_clu += amt_of_entry_per_clu;
         remain_entries -= amt_of_entry_per_clu;
     }

     // //////////////////////////////////////////////////////////////////////////////////////////////
     // write remaining entries towards last cluster

     m_disk->seek(ROOT_START_ADDR + (CLUSTER_SIZE * directory.dir_header.dir_entry_amt - remain_entries));
     m_disk->write((void*)&directory.dir_entries[directory.dir_header.dir_entry_amt], sizeof(dir_entry_t), remain_entries);
     fflush(m_disk->get_file());
     // /////////////////////////////////////////////////////////////////////////////////////////////
     // fill in fat_table

     m_fat_table[first_clu_index] = clu_list[0];
     for(int i = 1; i < num_of_clu_needed; i++)
         m_fat_table[clu_list[i] - 1] = m_fat_table[i];
     m_fat_table[num_of_clu_needed -1] = EOF_CLUSTER;
     // /////////////////////////////////////////////////////////////////////////////////////////////
     // free heap allocated memory and store unsaved structures onto disk

     free(clu_list);
     store_fat_table();
 }

 uint32_t FAT32::attain_clu() const noexcept {
     uint32_t rs = 0;
     for(int i = 0; i < CLUSTER_AMT; i++) {
         if(m_fat_table[i] == UNALLOCATED_CLUSTER) {
             rs = i;
             m_fat_table[rs] = ALLOCATED_CLUSTER;
             break;
         }
     }
     return rs;
 }

 uint32_t FAT32::n_free_clusters(const uint32_t& req) const noexcept {
     uint32_t amt = 0;
     for(int i = 0; i < CLUSTER_AMT; i++) {
         if(m_fat_table[i] == UNALLOCATED_CLUSTER)
            amt++;
         if(amt >= req)
             break;
     }
     return amt == req ? 1 : 0;
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

void FAT32::load() noexcept {

}
