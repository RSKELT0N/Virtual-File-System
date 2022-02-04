#include "../include/FAT32.h"

extern std::vector<std::string> split(const char* line, char sep) noexcept;

IFS::IFS() = default;
const uint32_t FAT32::CLUSTER_SIZE;
const uint32_t FAT32::CLUSTER_AMT;

FAT32::FAT32(const char* disk_name) {
    if (disk_name[0] == '\0')
        DISK_NAME = DEFAULT_DISK;
    else DISK_NAME = disk_name;

    std::string cmpl = "disks/" + std::string(DISK_NAME);
    FAT32::PATH_TO_DISK = cmpl.c_str();

    if (STORAGE_SIZE > (1ULL << 32)) {
        BUFFER << (LOG_str(Log::ERROR_, "maximum storage can only be 4gb"));
    }

    m_disk = new Disk();
    init();
}

FAT32::~FAT32() {
    free(m_fat_table);
    //m_disk->rm();
    delete (Disk*)m_disk;
    delete m_root;
    if (m_curr_dir != m_root)
        delete m_curr_dir;
    BUFFER << "Deleted FS\n";
}

void FAT32::init() noexcept {

    if (access(PATH_TO_DISK, F_OK) == -1)
        set_up();
    else load();
}

void FAT32::set_up() noexcept {
    define_superblock();
    define_fat_table();

    m_root = init_dir(0, 0, "root");
    m_curr_dir = m_root;

    create_disk();
    store_superblock();
    store_fat_table();
    store_dir(*m_root);
    
    BUFFER << (LOG_str(Log::INFO, "file system has been initialised."));
    
    print_super_block();
#if _DEBUG_
    print_fat_table();
#endif // _DEBUG_
}

void FAT32::create_disk() noexcept {
    m_disk->open(DISK_NAME, (const char*)"wb");
    m_disk->truncate(STORAGE_SIZE);
    rewind(((Disk*)m_disk)->get_file());

    m_disk->close();

    m_disk->open(DISK_NAME, "rb+");
    BUFFER << LOG_str(Log::INFO, "binary file '" + std::string(DISK_NAME) + "' has been created.");
}

void FAT32::define_superblock() noexcept {
    FAT32::metadata_t data{};
    strcpy(data.disk_name, DISK_NAME);
    data.cluster_size = CLUSTER_SIZE;
    data.disk_size = STORAGE_SIZE;
    data.cluster_n = CLUSTER_AMT;
    data.superblock_size = SUPERBLOCK_SIZE;
    data.fat_table_size = FAT_TABLE_SIZE;
    data.user_size = USER_SPACE;

    m_superblock.data = data;
    m_superblock.superblock_addr = SUPERBLOCK_START_ADDR;
    m_superblock.fat_table_addr = FAT_TABLE_START_ADDR;
    m_superblock.root_dir_addr = ROOT_START_ADDR;
}

void FAT32::define_fat_table() noexcept {
    m_fat_table = (uint32_t*)malloc(sizeof(uint32_t) * CLUSTER_AMT);
    memset((void*)m_fat_table, UNALLOCATED_CLUSTER, (size_t)FAT_TABLE_SIZE);
}

FAT32::dir_t* FAT32::init_dir(const uint32_t & start_cl, const uint32_t & parent_clu, const char* name) noexcept {
    dir_header_t hdr;
    auto* tmp = (dir_t*)malloc(sizeof(dir_t));
    strcpy(hdr.dir_name, name);
    hdr.start_cluster_index = start_cl;
    hdr.parent_cluster_index = parent_clu;
    hdr.dir_entry_amt = 2;

    tmp->dir_header = hdr;
    tmp->dir_entries = (dir_entry_t*)malloc(sizeof(dir_entry_t) * tmp->dir_header.dir_entry_amt);

    strcpy(tmp->dir_entries[0].dir_entry_name, ".");
    tmp->dir_entries[0].start_cluster_index = start_cl;
    tmp->dir_entries[0].is_directory = DIRECTORY;
    tmp->dir_entries[0].dir_entry_size = sizeof(tmp->dir_entries[0]);

    strcpy(tmp->dir_entries[1].dir_entry_name, "..");
    tmp->dir_entries[1].start_cluster_index = parent_clu;
    tmp->dir_entries[1].is_directory = DIRECTORY;
    tmp->dir_entries[1].dir_entry_size = sizeof(tmp->dir_entries[1]);

    return tmp;
}

void FAT32::store_superblock() noexcept {
    m_disk->seek(m_superblock.superblock_addr);
    m_disk->write((void*)&m_superblock, sizeof(m_superblock), 1);
    fflush(((Disk*)m_disk)->get_file());
}

void FAT32::store_fat_table() noexcept {
    m_disk->seek(m_superblock.fat_table_addr);
    m_disk->write(m_fat_table, sizeof(uint32_t), (uint32_t)CLUSTER_AMT);
    fflush(((Disk*)m_disk)->get_file());
}

void FAT32::store_dir(dir_t & directory)  noexcept {
    // ensuring header data is able to fit within a cluster size
    if (CLUSTER_SIZE < sizeof(directory.dir_header) || CLUSTER_SIZE < sizeof(dir_entry_t)) {
        BUFFER << (LOG_str(Log::ERROR_, "Insufficient memory to store header data/dir entry for directory"));
        return;
    }

    // //////////////////////////////////////////////////////////////////////////////////////////////
    // initialise variables for workout within cluster size

    uint16_t first_clu_entry_amt = (CLUSTER_SIZE - sizeof(directory.dir_header)) / sizeof(dir_entry_t);
    uint16_t remain_entries = (first_clu_entry_amt >= directory.dir_header.dir_entry_amt) ? 0 : (directory.dir_header.dir_entry_amt - (uint32_t)first_clu_entry_amt);
    uint16_t amt_of_entry_per_clu = 0;
    uint16_t entries_written = 0;

    // //////////////////////////////////////////////////////////////////////////////////////////////
    // get first cluster and store header data within
    if (!n_free_clusters(1)) {
        BUFFER << (LOG_str(Log::ERROR_, "amount of clusters needed is not valid"));
        return;
    }

    uint32_t first_clu_index = attain_clu();

    directory.dir_header.start_cluster_index = first_clu_index;
    directory.dir_entries[0].start_cluster_index = first_clu_index;

    if (remain_entries > 0)
        amt_of_entry_per_clu = CLUSTER_SIZE / sizeof(dir_entry_t);

    m_disk->seek(m_superblock.root_dir_addr + (first_clu_index * CLUSTER_SIZE));
    m_disk->write((void*)&directory.dir_header, sizeof(dir_header_t), 1);
    fflush(((Disk*)m_disk)->get_file());

    uint16_t num_of_clu_needed;
    if (remain_entries < amt_of_entry_per_clu)
        num_of_clu_needed = 1;
    else num_of_clu_needed = (uint16_t)((double)remain_entries / (double)amt_of_entry_per_clu);

    // //////////////////////////////////////////////////////////////////////////////////////////////
    // write entries in first cluster index
    for (int i = 0; i < min_(directory.dir_header.dir_entry_amt, first_clu_entry_amt); i++) {
        size_t addr_offset = (ROOT_START_ADDR + (first_clu_index * CLUSTER_SIZE) + sizeof(dir_header_t));

        m_disk->seek(addr_offset + (i * sizeof(dir_entry_t)));
        m_disk->write((void*)&directory.dir_entries[i], sizeof(dir_entry_t), 1);
        fflush(((Disk*)m_disk)->get_file());
        entries_written += 1;
    }
    // /////////////////////////////////////////////////////////////////////////////////////////////
    // add remaining entries within available clusters, up until last cluster

    if (remain_entries == 0) {
        m_fat_table[first_clu_index] = EOF_CLUSTER;
        store_fat_table();
        return;
    }

    if (remain_entries > amt_of_entry_per_clu) {
        if (remain_entries % amt_of_entry_per_clu > 0)
            num_of_clu_needed++;
    }

    if (!n_free_clusters(num_of_clu_needed)) {
        BUFFER << (LOG_str(Log::WARNING, "remaining entries cannot be stored due to insufficient cluster amount"));
        BUFFER << (LOG_str(Log::WARNING, "'" + std::string(directory.dir_header.dir_name) + "' directory cannot be stored within: '" + std::string(DISK_NAME) + "'"));
        m_fat_table[first_clu_index] = UNALLOCATED_CLUSTER;
        return;
    }

    uint32_t* clu_list = (uint32_t*)malloc(sizeof(uint32_t) * num_of_clu_needed);
    memset(clu_list, 0, num_of_clu_needed);

    for (int i = 0; i < num_of_clu_needed; i++)
        clu_list[i] = attain_clu();

    for (int i = 0; i < num_of_clu_needed - 1; i++) {
        size_t addr_offset = (ROOT_START_ADDR + (clu_list[i] * CLUSTER_SIZE));

        m_disk->seek(addr_offset);
        m_disk->write((void*)&directory.dir_entries[entries_written], sizeof(dir_entry_t), amt_of_entry_per_clu);
        fflush(((Disk*)m_disk)->get_file());
        remain_entries -= amt_of_entry_per_clu;
        entries_written += amt_of_entry_per_clu;
    }


    // //////////////////////////////////////////////////////////////////////////////////////////////
    // write remaining entries towards last cluster

    m_disk->seek(ROOT_START_ADDR + (CLUSTER_SIZE * clu_list[num_of_clu_needed - 1]));
    m_disk->write((void*)&directory.dir_entries[entries_written], sizeof(dir_entry_t), remain_entries);
    fflush(((Disk*)m_disk)->get_file());
    // /////////////////////////////////////////////////////////////////////////////////////////////
    // fill in fat_table

    m_fat_table[first_clu_index] = clu_list[0];
    for (int i = 0; i < num_of_clu_needed; i++)
        m_fat_table[clu_list[i]] = clu_list[i + 1];
    m_fat_table[clu_list[num_of_clu_needed - 1]] = EOF_CLUSTER;
    // /////////////////////////////////////////////////////////////////////////////////////////////
    // free heap allocated memory and store unsaved structures onto disk
    free(clu_list);
    store_fat_table();
}

void FAT32::save_dir(dir_t &directory) noexcept {
    std::vector<uint32_t> alloc_clu = get_list_of_clu(directory.dir_header.start_cluster_index);

    for (int i = 0; i < alloc_clu.size(); i++) {
        m_fat_table[alloc_clu[i]] = UNALLOCATED_CLUSTER;
    }

    store_dir(directory);
}

void FAT32::load() noexcept {
    BUFFER << (LOG_str(Log::INFO, "Loading disk into memory..."));
    m_disk->open(DISK_NAME, "rb+");
    load_superblock();
    define_fat_table();
    load_fat_table();
    m_root = read_dir(0);
    m_curr_dir = m_root;
    BUFFER << (LOG_str(Log::INFO, "Disk '" + std::string(DISK_NAME) + "' has been loaded"));
    print_super_block();
    print_fat_table();
}

void FAT32::load_superblock() noexcept {
    m_disk->seek(SUPERBLOCK_START_ADDR);
    m_disk->read((void*)&m_superblock, sizeof(superblock_t), 1);
}

void FAT32::load_fat_table() noexcept {
    m_disk->seek(m_superblock.fat_table_addr);
    m_disk->read(m_fat_table, sizeof(uint32_t), CLUSTER_AMT);
}

uint32_t FAT32::insert_dir(dir_t & curr_dir, const char* dir_name) noexcept {
    dir_t* tmp;
    uint32_t ret;

    tmp = init_dir(UNDEF_START_CLUSTER, curr_dir.dir_header.start_cluster_index, dir_name);
    store_dir(*tmp);

    add_new_entry(curr_dir, dir_name, tmp->dir_header.start_cluster_index, sizeof(dir_entry_t), 0x1);
    save_dir(curr_dir);

    ret = tmp->dir_header.start_cluster_index;
    delete tmp;
    return ret;
}

FAT32::dir_t* FAT32::read_dir(const uint32_t & start_clu) noexcept {
    if (m_fat_table[start_clu] == UNALLOCATED_CLUSTER) {
        BUFFER << (LOG_str(Log::WARNING, "specified cluster has not been allocated"));
        return nullptr;
    }

    //tmp dir_t*, return
    dir_t* ret = (dir_t*)malloc(sizeof(dir_t));

    uint32_t dir_start_addr = ROOT_START_ADDR + (CLUSTER_SIZE * start_clu);

    //attain dir_header
    m_disk->seek(dir_start_addr);
    m_disk->read((void*)&ret->dir_header, sizeof(dir_header_t), 1);
    fflush(((Disk*)m_disk)->get_file());

    uint16_t first_clu_entry_amt = (CLUSTER_SIZE - sizeof(ret->dir_header)) / sizeof(dir_entry_t);
    uint16_t remain_entries = (first_clu_entry_amt >= ret->dir_header.dir_entry_amt) ? 0 : (ret->dir_header.dir_entry_amt - (uint32_t)first_clu_entry_amt);
    uint16_t entries_read = 0;

    //allocate memory to ret(dir_t) entries due to dir header data.
    ret->dir_entries = (dir_entry_t*)malloc(sizeof(dir_entry_t) * ret->dir_header.dir_entry_amt);

    for (int i = 0; i < min_(first_clu_entry_amt, ret->dir_header.dir_entry_amt); i++) {
        size_t addr_offset = dir_start_addr + sizeof(dir_header_t);

        m_disk->seek(addr_offset + (i * sizeof(dir_entry_t)));
        m_disk->read((void*)&ret->dir_entries[i], sizeof(dir_entry_t), 1);
        fflush(((Disk*)m_disk)->get_file());
        entries_read += 1;
    }


    uint32_t amt_of_entries_per_clu;
    if (remain_entries > 0) {
        amt_of_entries_per_clu = CLUSTER_SIZE / sizeof(dir_entry_t);
    }
    else return ret;

    uint32_t amt_of_clu_used = remain_entries / amt_of_entries_per_clu;

    if (remain_entries > amt_of_entries_per_clu) {
        if (remain_entries % amt_of_entries_per_clu > 0)
            amt_of_clu_used++;
    }

    //move towards next clu
    uint32_t curr_clu = start_clu;
    curr_clu = m_fat_table[curr_clu];

    for (int i = 0; i < amt_of_clu_used - 1; i++) {
        size_t addr_offset = ROOT_START_ADDR + (CLUSTER_SIZE * curr_clu);

        m_disk->seek(addr_offset);
        m_disk->read((void*)&ret->dir_entries[entries_read], sizeof(dir_entry_t), amt_of_entries_per_clu);
        fflush(((Disk*)m_disk)->get_file());
        entries_read += amt_of_entries_per_clu;
        curr_clu = m_fat_table[curr_clu];
    }

    uint32_t remain_entries_in_lst_clu = abs_(ret->dir_header.dir_entry_amt, entries_read);

    m_disk->seek(ROOT_START_ADDR + (CLUSTER_SIZE * curr_clu));
    m_disk->read((void*)&ret->dir_entries[entries_read], sizeof(dir_entry_t), remain_entries_in_lst_clu);
    fflush(((Disk*)m_disk)->get_file());

    return ret;
}

std::string FAT32::read_file(dir_t & dir, const char* entry_name) noexcept {
    dir_entry_t* entry_ptr = find_entry(dir, entry_name, 1);

    if (m_fat_table[entry_ptr->start_cluster_index] == UNALLOCATED_CLUSTER) {
        BUFFER << (LOG_str(Log::WARNING, "cluster specified has not been allocated, file could not be read"));
        return "";
    }

    uint32_t amt_of_clu_used = 0;
    size_t entry_size = entry_ptr->dir_entry_size;
    char buffer[entry_size];
    uint32_t data_read = 0;

    if (entry_size <= CLUSTER_SIZE)
        amt_of_clu_used = 1;
    else amt_of_clu_used = entry_size / CLUSTER_SIZE;

    if (entry_size > CLUSTER_SIZE) {
        if (entry_size % CLUSTER_SIZE > 0)
            amt_of_clu_used++;
    }

    if (amt_of_clu_used == 1) {
        m_disk->seek(ROOT_START_ADDR + (CLUSTER_SIZE * entry_ptr->start_cluster_index));
        m_disk->read(buffer, sizeof(char), entry_size);
        fflush(((Disk*)m_disk)->get_file());

        data_read += entry_size;

        return buffer;
    }

    uint32_t curr_clu = entry_ptr->start_cluster_index;

    for (int i = 0; i < amt_of_clu_used - 1; i++) {
        size_t addr_offset = ROOT_START_ADDR + (CLUSTER_SIZE * curr_clu);

        m_disk->seek(addr_offset);
        m_disk->read(buffer + data_read, sizeof(char), CLUSTER_SIZE);
        fflush(((Disk*)m_disk)->get_file());
        curr_clu = m_fat_table[curr_clu];
        data_read += CLUSTER_SIZE;
    }

    uint32_t remaining_data = abs_(entry_size, data_read);

    m_disk->seek(ROOT_START_ADDR + (CLUSTER_SIZE * curr_clu));
    m_disk->read(buffer + data_read, sizeof(char), remaining_data);
    fflush(((Disk*)m_disk)->get_file());
    
    return buffer;
}


int32_t FAT32::store_file(const char* data) noexcept {

    uint32_t sdata = strlen(data);
    uint32_t first_cluster = 0;
    size_t data_read = 0;
    
    uint32_t amt_of_clu_needed = 0;

    if (sdata < CLUSTER_SIZE)
        amt_of_clu_needed = 1;
    else amt_of_clu_needed = sdata / CLUSTER_SIZE;

     if (sdata > CLUSTER_SIZE) {
        if (sdata % CLUSTER_SIZE > 0)
            amt_of_clu_needed++;
    }

    if (amt_of_clu_needed == 1) {
        first_cluster = attain_clu();

        m_disk->seek(ROOT_START_ADDR + (CLUSTER_SIZE * first_cluster));
        m_disk->write(data, sizeof(char), sdata);
        data_read += sdata;
        fflush(((Disk*)m_disk)->get_file());

        m_fat_table[first_cluster] = EOF_CLUSTER;
        return first_cluster;
    }

    if (!n_free_clusters(amt_of_clu_needed)) {
        BUFFER << (LOG_str(Log::WARNING, "amount of cluster needed isn't available to store file"));
        return -1;
    }

    uint32_t* clu_list = (uint32_t*)malloc(sizeof(uint32_t) * amt_of_clu_needed);
    memset(clu_list, 0, amt_of_clu_needed);

    for (int i = 0; i < amt_of_clu_needed; i++)
        clu_list[i] = attain_clu();

    first_cluster = clu_list[0];

    for (int i = 0; i < amt_of_clu_needed - 1; i++) {
        size_t off_adr = ROOT_START_ADDR + (CLUSTER_SIZE * clu_list[i]);

        m_disk->seek(off_adr);
        m_disk->write(data+data_read, CLUSTER_SIZE, 1);
        fflush(((Disk*)m_disk)->get_file());

        data_read += CLUSTER_SIZE;
    }

    size_t remaining_data = abs_(sdata, data_read);

    m_disk->seek(ROOT_START_ADDR + (CLUSTER_SIZE * clu_list[amt_of_clu_needed - 1]));
    m_disk->write(data+data_read, remaining_data, 1);
    fflush(((Disk*)m_disk)->get_file());

    m_fat_table[first_cluster] = clu_list[0];
    for (int i = 0; i < amt_of_clu_needed; i++)
        m_fat_table[clu_list[i]] = clu_list[i + 1];
    m_fat_table[clu_list[amt_of_clu_needed - 1]] = EOF_CLUSTER;

    free(clu_list);
    return first_cluster;
}

void FAT32::insert_int_file(dir_t& dir, const char* buffer, const char* name) noexcept {
    uint32_t sbuffer = strlen(buffer);
    uint32_t start_clu = store_file(buffer);

    if (start_clu == -1) {
        BUFFER << (LOG_str(Log::WARNING, "file could not be stored"));
        return;
    }

    add_new_entry(dir, name, start_clu, sbuffer, 0);
    save_dir(dir);
}

void FAT32::insert_ext_file(dir_t & curr_dir, const char* path, const char* name) noexcept {
    if (access(path, F_OK) == -1) {
        BUFFER << (LOG_str(Log::WARNING, "file specified does not exist"));
        return;
    }

    std::string buffer = get_ext_file_buffer(path);
    uint32_t start_clu = store_file(buffer.c_str());

    if (start_clu == -1) {
        BUFFER << (LOG_str(Log::WARNING, "file could not be stored"));
        return;
    }

    add_new_entry(curr_dir, name, start_clu, buffer.size(), 0);
    save_dir(curr_dir);
}

void FAT32::delete_entry(dir_entr_ret_t& entry) noexcept {

    std::vector<uint32_t> entry_alloc_clu = get_list_of_clu(entry.m_entry->start_cluster_index);

    for (int i = 0; i < entry_alloc_clu.size(); i++) {
        m_fat_table[entry_alloc_clu[i]] = UNALLOCATED_CLUSTER;
    }

    rm_entr_mem(*(entry.m_dir), entry.m_entry->dir_entry_name);
}

void FAT32::delete_dir(dir_t& dir) noexcept {
    for(int i = dir.dir_header.dir_entry_amt - 1; i >= 2; i--) { // i >= 2, as 0 = '.' and 1 = '..'
        dir_t* tmp = nullptr;
        if(dir.dir_entries[i].is_directory) {
            tmp = read_dir(dir.dir_entries[i].start_cluster_index);
            delete_dir(*tmp);
            delete tmp;
        }
        dir_entr_ret_t entr = {&dir, &dir.dir_entries[i]};
        delete_entry(entr);
    }
}

FAT32::dir_entr_ret_t* FAT32::parsePath(std::vector<std::string>&path, uint8_t shd_exst) noexcept {
    FAT32::dir_entr_ret_t* ret = (dir_entr_ret_t*)malloc(sizeof(dir_entr_ret_t));

    dir_t* curr_dir = m_curr_dir;
    dir_entry_t* tmp_entr;

    for (int i = 0; i < path.size() - 1; i++) {
        dir_entry_t* tmp_entr = find_entry(*curr_dir, path[i].c_str(), 0x1);

        if (tmp_entr == nullptr && 0x1) {
            delete ret;
            return nullptr;
        }

        if (curr_dir != m_curr_dir)
            delete curr_dir;
        curr_dir = read_dir(tmp_entr->start_cluster_index);

    }

    tmp_entr = find_entry(*curr_dir, path[path.size() - 1].c_str(), shd_exst);

    if (!tmp_entr && shd_exst == 1) {
        delete ret;
        return nullptr;
    }

    if(tmp_entr && shd_exst == 0) {
        delete ret;
        return nullptr;
    }

    ret->m_dir = curr_dir;
    ret->m_entry = tmp_entr;

    return ret;
}

uint32_t FAT32::attain_clu() const noexcept {
    uint32_t rs = 0;
    for (int i = 0; i < CLUSTER_AMT; i++) {
        if (m_fat_table[i] == UNALLOCATED_CLUSTER) {
            rs = i;
            m_fat_table[rs] = ALLOCATED_CLUSTER;
            break;
        }
    }
    return rs;
}

uint32_t FAT32::n_free_clusters(const uint32_t & req) const noexcept {
    uint32_t amt = 0;
    for (int i = 0; i < CLUSTER_AMT; i++) {
        if (m_fat_table[i] == UNALLOCATED_CLUSTER)
            amt++;
        if (amt >= req)
            break;
    }
    return amt == req ? 1 : 0;
}

std::vector<uint32_t> FAT32::get_list_of_clu(const uint32_t & start_clu) noexcept {
    std::vector<uint32_t> alloc_clu;

    uint32_t curr_clu = start_clu;
    alloc_clu.push_back(curr_clu);

    while (1) {
        uint32_t next_clu = m_fat_table[curr_clu];
        if (next_clu == EOF_CLUSTER)
            break;
        alloc_clu.push_back(next_clu);
        curr_clu = next_clu;
    }
    return alloc_clu;
}

void FAT32::rm_entr_mem(dir_t & dir, const char* name) noexcept {

    dir_entry_t* tmp = (dir_entry_t*)malloc(sizeof(dir_entry_t) * (dir.dir_header.dir_entry_amt - 1));

    for (int i = 0; i < dir.dir_header.dir_entry_amt; i++) {
        if (strcmp(dir.dir_entries[i].dir_entry_name, name) == 0) {
            for (int j = i; j < dir.dir_header.dir_entry_amt - 1; j++) {
                tmp[j] = dir.dir_entries[j + 1];
            }
            break;
        }
        tmp[i] = dir.dir_entries[i];
    }

    delete[] dir.dir_entries;
    dir.dir_entries = tmp;
    dir.dir_header.dir_entry_amt--;
}

void FAT32::add_new_entry(dir_t& curr_dir, const char* name, const uint32_t& start_clu, const uint32_t& size, const uint8_t& is_dir) noexcept {

    curr_dir.dir_header.dir_entry_amt += 1;
    dir_entry_t* tmp_entries = curr_dir.dir_entries;
    curr_dir.dir_entries = (dir_entry_t*)malloc(sizeof(dir_entry_t) * curr_dir.dir_header.dir_entry_amt);

    for (int i = 0; i < curr_dir.dir_header.dir_entry_amt - 1; i++)
        curr_dir.dir_entries[i] = tmp_entries[i];

    strcpy(curr_dir.dir_entries[curr_dir.dir_header.dir_entry_amt - 1].dir_entry_name, name);
    curr_dir.dir_entries[curr_dir.dir_header.dir_entry_amt - 1].start_cluster_index = start_clu;
    curr_dir.dir_entries[curr_dir.dir_header.dir_entry_amt - 1].is_directory = is_dir;
    curr_dir.dir_entries[curr_dir.dir_header.dir_entry_amt - 1].dir_entry_size = size;

    delete tmp_entries;
}

void FAT32::cp_dir(dir_t& src, dir_t& dst) noexcept {
    for(int i = src.dir_header.dir_entry_amt - 1; i >= 2; i--) { // i >= 2, as 0 = '.' and 1 = '..'
        dir_t* tmp = nullptr;
        if(src.dir_entries[i].is_directory) {
            tmp = read_dir(src.dir_entries[i].start_cluster_index);
            uint32_t cp_clu = insert_dir(dst, tmp->dir_header.dir_name);
            dir_t* dir_cp = read_dir(cp_clu);

            cp_dir(*tmp, *dir_cp);
            delete tmp;
            delete dir_cp;
            continue;
        }
        std::string buffer = read_file(src, src.dir_entries[i].dir_entry_name);
        insert_int_file(dst, buffer.c_str(), src.dir_entries[i].dir_entry_name);
    }
}

void FAT32::mv(std::vector<std::string>& tokens) noexcept {
    std::vector<std::string> parts = split(tokens[0].c_str(), '/');
    dir_entr_ret_t* src = parsePath(parts, 0x1);

    parts = split(tokens[1].c_str(), '/');
    dir_entr_ret_t* dst = parsePath(parts, 0x0);
    const char* entr_name = parts[parts.size() - 1].c_str();

    if(!src || !dst) {
        BUFFER << (LOG_str(Log::WARNING, "Either src or dst specified is invalid"));
        return;
    }

    if(src->m_entry->is_directory) {
        add_new_entry(*dst->m_dir, entr_name, src->m_entry->start_cluster_index, src->m_entry->dir_entry_size, 0x1);

        dir_t* mv_dir = read_dir(dst->m_dir->dir_entries[dst->m_dir->dir_header.dir_entry_amt - 1].start_cluster_index);
        mv_dir->dir_entries[1].start_cluster_index = dst->m_dir->dir_header.start_cluster_index;
        save_dir(*mv_dir);
        delete mv_dir;
    } else
        add_new_entry(*dst->m_dir, entr_name, src->m_entry->start_cluster_index, src->m_entry->dir_entry_size, 0x0);

    rm_entr_mem(*src->m_dir, src->m_entry->dir_entry_name);

    save_dir(*src->m_dir);
    save_dir(*dst->m_dir);
    delete src;
    delete dst;
}

void FAT32::cp(const char* src, const char* dst) noexcept {
    std::vector<std::string> parts = split(src, '/');
    dir_entr_ret_t* dsrc = parsePath(parts, 0x1);

    parts = split(dst, '/');
    dir_entr_ret_t* ddst = parsePath(parts, 0x0);
    const char* entr_name = parts[parts.size() - 1].c_str();

    if(!dsrc || !ddst) {
        BUFFER << (LOG_str(Log::WARNING, "Either src or dst specified is invalid"));
        return;
    }

    if(dsrc->m_entry->is_directory) {
        dir_t* src_dir = read_dir(dsrc->m_entry->start_cluster_index);
        uint32_t dir_clu = insert_dir(*ddst->m_dir, entr_name);
        dir_t* dst_dir = read_dir(dir_clu);

        cp_dir(*src_dir, *dst_dir);

        delete src_dir;
        delete dst_dir;
    } else {
        std::string buffer = read_file(*dsrc->m_dir, dsrc->m_entry->dir_entry_name);
        insert_int_file(*ddst->m_dir, buffer.c_str(), entr_name);
    }

    save_dir(*ddst->m_dir);

    delete dsrc;
    delete ddst;
}

void FAT32::cp_ext(const char* src, const char* dst) noexcept {
    std::vector<std::string> parts = split(dst, '/');
    dir_entr_ret_t* ddst = parsePath(parts, 0x0);
    const char* entr_name = parts[parts.size() - 1].c_str();

    if(!dst) {
        BUFFER << (LOG_str(Log::WARNING, "dst specified is invalid"));
        return;
    }

    insert_ext_file(*ddst->m_dir, src, entr_name);
    delete ddst;
}

void FAT32::mkdir(const char* dir) noexcept {
    std::vector<std::string> tokens = split(dir, '/');
    FAT32::dir_entr_ret_t* ret = parsePath(tokens, 0x0);

    if (!ret) {
        BUFFER << (LOG_str(Log::WARNING, "Path specified is invalid"));
        return;
    }
    insert_dir(*ret->m_dir, tokens[tokens.size() - 1].c_str());
    delete ret;
}

void FAT32::cd(const char* pth) noexcept {
    std::vector<std::string> tokens = split(pth, '/');
    FAT32::dir_entr_ret_t* ret = parsePath(tokens, 0x1);

    if (!ret) {
        BUFFER << (LOG_str(Log::WARNING, "entry does not exist"));
        return;
    }

    if (!ret->m_entry->is_directory) {
        BUFFER << (LOG_str(Log::WARNING, "entry '" + std::string(ret->m_entry->dir_entry_name) + "' is not a directory"));
        return;
    }

    if (m_curr_dir != m_root)
        delete m_curr_dir;

    m_curr_dir = read_dir(ret->m_entry->start_cluster_index);
    delete ret;
}

void FAT32::rm(std::vector<std::string>&tokens) noexcept {
    for (int i = 0; i < tokens.size(); i++) {
        std::vector<std::string> parts = split(tokens[i].c_str(), '/');
        dir_entr_ret_t* entry = parsePath(parts, 0x1);

        if (entry == nullptr) {
            BUFFER << (LOG_str(Log::WARNING, "Path is not valid, either directory/file's specified are non-existant"));
            delete entry;
            return;
        }

        if(!entry->m_entry->is_directory) {
            delete_entry(*entry);
        } else {
            dir_t* tmp = read_dir(entry->m_entry->start_cluster_index);
            delete_dir(*tmp);
            delete_entry(*entry);
            delete tmp;
        }
        save_dir(*entry->m_dir);
        delete entry;
    }
}

void FAT32::touch(std::vector<std::string>& parts, const char* buffer) noexcept {
    for(int i = 0; i < parts.size(); i++) {
        std::vector<std::string> tokens = split(parts[i].c_str(), '/');
        dir_entr_ret_t* entr = parsePath(tokens, 0x0);
        const char* init_file_name = tokens[tokens.size() - 1].c_str();

        if(!entr) {
            BUFFER << (LOG_str(Log::WARNING, "Path specified is invalid"));
            delete entr;
            return;
        }

        insert_int_file(*entr->m_dir, buffer, init_file_name);

        delete entr;
    }
}

void FAT32::cat(const char* path) noexcept {
    std::vector<std::string> tokens = split(path, '/');
    dir_entr_ret_t* entr = parsePath(tokens, 0x1);

    if(!entr) {
        BUFFER << (LOG_str(Log::WARNING, "Path specified is invalid"));
        return;
    }

    std::string buffer = read_file(*entr->m_dir, tokens[tokens.size() - 1].c_str());
    BUFFER << ("\n%s\n%s\n%s\n\n", tokens[tokens.size() - 1].c_str(), "------------", buffer.c_str());
    BUFFER << "\n";
}

void FAT32::ls() noexcept {
    BUFFER << ("\n");
    print_dir(*m_curr_dir);
}

FAT32::dir_entry_t* FAT32::find_entry(dir_t & dir, const char* entry, uint8_t shd_exst) const noexcept {
    dir_entry_t* ret = nullptr;
    for (int i = 0; i < dir.dir_header.dir_entry_amt; i++) {
        if (strcmp(dir.dir_entries[i].dir_entry_name, entry) == 0) {
            ret = &dir.dir_entries[i];
            break;
        }
    }

    if (shd_exst == 1 && ret == nullptr) {
        BUFFER << (LOG_str(Log::WARNING, "entry '" + std::string(entry) + "', could not be found"));
    } else if (shd_exst == 0 && ret != nullptr) {
        BUFFER << (LOG_str(Log::WARNING, "entry '" + std::string(entry) + "', already exists"));
    }

    return ret;
}

void FAT32::print_super_block() const noexcept {
    char buffer[400];
    BUFFER << "\n   Super block\n ---------------\n\n";

    BUFFER << "  meta data\n-------------\n";
    BUFFER << " -> Disk:            " << m_superblock.data.disk_name << "\n";
    BUFFER << " -> Disk size:       " << m_superblock.data.disk_size << "\n";
    BUFFER << " -> Superblock size: " << m_superblock.data.superblock_size << "\n";
    BUFFER << " -> Fat table size:  " << m_superblock.data.fat_table_size << "\n";
    BUFFER << " -> User space:      " << m_superblock.data.user_size << "\n";
    BUFFER << " -> Cluster size:    " << m_superblock.data.cluster_size << "\n";
    BUFFER << " -> Cluster amount:  " << m_superblock.data.cluster_n << "\n";

    BUFFER << "\n  Address space\n-----------------\n";


    sprintf(buffer, " -> [superblock : 0x%.8x]\n", m_superblock.superblock_addr);
    sprintf(buffer + strlen(buffer), " -> [fat_table  : 0x%.8x]\n", m_superblock.fat_table_addr);
    sprintf(buffer + strlen(buffer), " -> [user_space : 0x%.8x]\n", m_superblock.root_dir_addr);
    sprintf(buffer + strlen(buffer), "%s\n%s\n", "-----------------", "    End");

    BUFFER << (buffer);
}

void FAT32::print_fat_table() const noexcept {
    char buffer[1024 * 4];
    sprintf(buffer, "\n%s%s\n", "    Fat table\n", " --------------");
    for (int i = 0; i < CLUSTER_AMT; i++) {
        if(strlen(buffer) > (1024 * 4)) {
            BUFFER << LOG_str(Log::WARNING, "Buffer size isn't large enough print directory");
            return;
        }
        sprintf(buffer + strlen(buffer), "[%d : 0x%.8x]\n", i, m_fat_table[i]);
    }
    sprintf(buffer + strlen(buffer), "%s\n%s\n\n", "--------------", "    End");

    BUFFER << (buffer);
}

void FAT32::print_dir(dir_t & dir) const noexcept {
    char buffer[1024 * 4];
    if ((dir_t*)&dir == NULL) {
        BUFFER << (LOG_str(Log::WARNING, "specified directory to be printed is null"));
        return;
    }

    sprintf(buffer, "Directory:        %s\n", dir.dir_header.dir_name);
    sprintf(buffer + strlen(buffer), "Start cluster:    %d\n", dir.dir_header.start_cluster_index);
    sprintf(buffer + strlen(buffer), "Parent cluster:   %d\n", dir.dir_header.parent_cluster_index);
    sprintf(buffer + strlen(buffer), "Entry amt:        %d\n", dir.dir_header.dir_entry_amt);

    sprintf(buffer + strlen(buffer), "\n %s%4s%s%4s%s\n%s\n", "size", "", "start cluster", "", "name", "-------------------------------");

    for (int i = 0; i < dir.dir_header.dir_entry_amt; i++) {
        if(strlen(buffer) > (1024 * 4)) {
            BUFFER << LOG_str(Log::WARNING, "Buffer size isn't large enough print FAT table");
            return;
        }
        sprintf(buffer + strlen(buffer), "%05db%8s%02d%10s%s\n", dir.dir_entries[i].dir_entry_size, "", dir.dir_entries[i].start_cluster_index, "", dir.dir_entries[i].dir_entry_name);
    }
    sprintf(buffer + strlen(buffer), "-------------------------------");
    sprintf(buffer + strlen(buffer), "\n");

    BUFFER << (buffer);
}
