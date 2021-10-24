#include "FAT32.h"

 IFS::IFS() = default;

 FAT32::FAT32(const char *disk_name) {
     if(disk_name[0] == '\0')
         DISK_NAME = DEFAULT_DISK;
     else DISK_NAME = disk_name;

     if(STORAGE_SIZE > (1ULL << 32)) {
         LOG(Log::ERROR, "maximum storage can only be 4gb");
     }

     m_disk      = new Disk();
     m_fat_table = (uint32_t *)malloc(sizeof(uint32_t) * CLUSTER_AMT);
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
     m_root = init_dir(0, 0, "root");

     create_disk();
     store_superblock();
     store_fat_table();
     store_dir(*m_root);
     insert_dir(*m_root, "Documents");
     print_fat_table();
     dir_t* root = read_dir(0);
     print_dir(*root);
//     dir_t* doc = read_dir(1);
//     print_dir(*doc);
     //LOG(Log::INFO, "file system has been initialised.");
 }

 void FAT32::create_disk() noexcept {
     m_disk->open(DISK_NAME, (const char*)"wb");
     m_disk->truncate(STORAGE_SIZE);
     rewind(m_disk->get_file());

     m_disk->close();

     m_disk->open(DISK_NAME, "rb+");
     //LOG(Log::INFO, "binary file '" + std::string(DISK_NAME) + "' has been created.");
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
     m_disk->write(m_fat_table, sizeof(uint32_t), (uint32_t)CLUSTER_AMT);
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
     uint16_t entries_written = 0;

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

     uint16_t num_of_clu_needed;
     if(remain_entries < amt_of_entry_per_clu)
         num_of_clu_needed = 1;
     else num_of_clu_needed = (uint16_t)((double)remain_entries / (double)amt_of_entry_per_clu);

     // //////////////////////////////////////////////////////////////////////////////////////////////
     // write entries in first cluster index
     for(int i = 0; i < min(directory.dir_header.dir_entry_amt, first_clu_entry_amt); i++) {
         size_t addr_offset = (ROOT_START_ADDR + (first_clu_index * CLUSTER_SIZE) + sizeof(dir_header_t));

         m_disk->seek(addr_offset + (i * sizeof(dir_entry_t)));
         m_disk->write((void*)&directory.dir_entries[i], sizeof(dir_entry_t), 1);
         fflush(m_disk->get_file());
         entries_written += 1;
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

     uint32_t nxt_clu = entries_written;

     for(int i = 0; i < num_of_clu_needed - 1; i++) {
         size_t addr_offset = (ROOT_START_ADDR + (clu_list[i] * CLUSTER_SIZE) + sizeof(dir_header_t));

         m_disk->seek(addr_offset + (nxt_clu * sizeof(dir_entry_t)));
         m_disk->write((void*)&directory.dir_entries[nxt_clu], sizeof(dir_entry_t), amt_of_entry_per_clu);
         fflush(m_disk->get_file());
         nxt_clu += amt_of_entry_per_clu;
         remain_entries -= amt_of_entry_per_clu;
         entries_written += amt_of_entry_per_clu;
     }



     // //////////////////////////////////////////////////////////////////////////////////////////////
     // write remaining entries towards last cluster

     m_disk->seek(ROOT_START_ADDR + (CLUSTER_SIZE * clu_list[num_of_clu_needed - 1]));
     m_disk->write((void*)&directory.dir_entries[directory.dir_header.dir_entry_amt - 1], sizeof(dir_entry_t), remain_entries);
     fflush(m_disk->get_file());
     // /////////////////////////////////////////////////////////////////////////////////////////////
     // fill in fat_table

     m_fat_table[first_clu_index] = clu_list[0];
     for(int i = 0; i < num_of_clu_needed; i++)
         m_fat_table[clu_list[i]] = clu_list[i + 1];
     m_fat_table[clu_list[num_of_clu_needed - 1]] = EOF_CLUSTER;
     // /////////////////////////////////////////////////////////////////////////////////////////////
     // free heap allocated memory and store unsaved structures onto disk

     free(clu_list);
     store_fat_table();
 }

 void FAT32::insert_dir(dir_t& curr_dir, const char *dir_name) noexcept {
     dir_t* tmp;

     tmp = init_dir(UNDEF_START_CLUSTER, curr_dir.dir_header.start_cluster_index, dir_name);
     store_dir(*tmp);

     curr_dir.dir_header.dir_entry_amt += 1;
     dir_entry_t* tmp_entries = curr_dir.dir_entries;
     curr_dir.dir_entries = (dir_entry_t*)malloc(sizeof(dir_entry_t) * curr_dir.dir_header.dir_entry_amt);

     for(int i = 0; i < curr_dir.dir_header.dir_entry_amt - 1; i++) {
         curr_dir.dir_entries[i] = tmp_entries[i];
     }
     strcpy(curr_dir.dir_entries[curr_dir.dir_header.dir_entry_amt-1].dir_entry_name, dir_name);
     curr_dir.dir_entries[curr_dir.dir_header.dir_entry_amt-1].start_cluster_index = tmp->dir_header.start_cluster_index;
     curr_dir.dir_entries[curr_dir.dir_header.dir_entry_amt-1].is_directory = 1;
     curr_dir.dir_entries[curr_dir.dir_header.dir_entry_amt-1].dir_entry_size = sizeof(dir_t);

     std::vector<uint32_t> alloc_clu = get_list_of_clu(curr_dir.dir_header.start_cluster_index);

     for(int i = 0; i < alloc_clu.size(); i++) {
         m_fat_table[alloc_clu[i]] = UNALLOCATED_CLUSTER;
     }

     store_dir(curr_dir);
     delete tmp;
     delete tmp_entries;
 }

 FAT32::dir_t* FAT32::read_dir(const uint32_t& start_clu) noexcept {
     if(m_fat_table[start_clu] == UNALLOCATED_CLUSTER) {
         LOG(Log::WARNING, "specified cluster has not been allocated");
         return nullptr;
     }

     //tmp dir_t*, return
     dir_t* ret = (dir_t*)malloc(sizeof(dir_t));

     uint32_t dir_start_addr = ROOT_START_ADDR + (CLUSTER_SIZE * start_clu);

     //attain dir_header
     m_disk->seek(dir_start_addr);
     m_disk->read((void*)&ret->dir_header, sizeof(dir_header_t), 1);
     fflush(m_disk->get_file());

     uint16_t first_clu_entry_amt = (CLUSTER_SIZE - sizeof(ret->dir_header)) / sizeof(dir_entry_t);
     uint16_t remain_entries  = (first_clu_entry_amt >= ret->dir_header.dir_entry_amt) ? 0 : (ret->dir_header.dir_entry_amt - (uint32_t)first_clu_entry_amt);
     uint16_t entries_read = 0;

     //allocate memory to ret(dir_t) entries due to dir header data.
     ret->dir_entries = (dir_entry_t*)malloc(sizeof(dir_entry_t) * ret->dir_header.dir_entry_amt);

     for(int i = 0; i < min(first_clu_entry_amt, ret->dir_header.dir_entry_amt); i++) {
         size_t addr_offset = dir_start_addr + sizeof(dir_header_t);

         m_disk->seek(addr_offset + (i * sizeof(dir_entry_t)));
         m_disk->read((void*)&ret->dir_entries[i], sizeof(dir_entry_t), 1);
         fflush(m_disk->get_file());
         entries_read += 1;
     }


     uint32_t amt_of_entries_per_clu;
     if(remain_entries > 0) {
         amt_of_entries_per_clu = CLUSTER_SIZE / sizeof(dir_entry_t);
     } else return ret;

     uint32_t amt_of_clu_used = remain_entries / amt_of_entries_per_clu;

     if(remain_entries % amt_of_entries_per_clu > 0)
         amt_of_clu_used += 1;

     //move towards next clu
     uint32_t curr_clu = start_clu;
     curr_clu = m_fat_table[curr_clu];

     for(int i = 0; i < amt_of_clu_used - 1; i++) {
         size_t addr_offset = ROOT_START_ADDR + (CLUSTER_SIZE * curr_clu);

         m_disk->seek(addr_offset + (entries_read * sizeof(dir_entry_t)));
         m_disk->read((void*)&ret->dir_entries[entries_read], sizeof(dir_entry_t), amt_of_entries_per_clu);
         fflush(m_disk->get_file());
         entries_read += amt_of_entries_per_clu;
         curr_clu = m_fat_table[curr_clu];
     }

     uint32_t remain_entries_in_lst_clu = entries_read - remain_entries;

     m_disk->seek(ROOT_START_ADDR + (CLUSTER_SIZE * curr_clu));
     m_disk->read((void*)&ret->dir_entries[entries_read], sizeof(dir_entry_t), remain_entries_in_lst_clu);
     fflush(m_disk->get_file());

     return ret;
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

 std::vector<uint32_t> FAT32::get_list_of_clu(const uint32_t &start_clu) noexcept {
     std::vector<uint32_t> alloc_clu;

     uint32_t curr_clu = start_clu;
     alloc_clu.push_back(curr_clu);

     while(1) {
         uint32_t next_clu = m_fat_table[curr_clu];
         if(next_clu == EOF_CLUSTER)
             break;
         alloc_clu.push_back(next_clu);
         curr_clu = next_clu;
     }
     return alloc_clu;
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

void FAT32::print_fat_table() const noexcept {
     printf("\n%s%s\n", "    Fat table\n", " --------------");
     for(int i = 0; i < CLUSTER_AMT; i++) {
         printf("[%d : 0x%.8x]\n", i, m_fat_table[i]);
     }
 }

 void FAT32::print_dir(dir_t &dir) const noexcept {
     printf("Directory:        %s\n", dir.dir_header.dir_name);
     printf("Start cluster:    %d\n", dir.dir_header.start_cluster_index);
     printf("Parent cluster:   %d\n", dir.dir_header.parent_cluster_index);
     printf("Entry amt:        %d\n", dir.dir_header.dir_entry_amt);

     printf(" %s%4s%s%4s%s\n%s\n", "size", "", "start cluster", "", "name", "-------------------------------");

     for(int i = 0; i < dir.dir_header.dir_entry_amt; i++) {
         printf("%db%10s%d%12s%s\n", dir.dir_entries[i].dir_entry_size, "", dir.dir_entries[i].start_cluster_index, "", dir.dir_entries[i].dir_entry_name);
     }
 }
