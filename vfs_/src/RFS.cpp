#include "../include/RFS.h"

char* generate_hash() noexcept {
    char* hash = (char*)malloc(sizeof(char) * CFG_PACKET_HASH_SIZE);


    for(int i = 0; i < CFG_PACKET_HASH_SIZE; i++) {
        int r = rand() % 122 + 36;
        hash[i] = (char)r;
    }
    return hash;
}

RFS::pcontainer_t* RFS::generate_container(uint8_t cmd, std::vector<std::string>& args, std::string payload) noexcept {
        pcontainer_t* con = new pcontainer_t;
        char* hash = generate_hash();
        memcpy(con->info.hash, hash, CFG_PACKET_HASH_SIZE);
        free(hash);

        std::string flags = {};

        for(const auto& a : args)
            flags += a + " ";

        flags[flags.size() - 1] = '\0';
        
        size_t load_size = payload.size();
        int amount_of_payloads = 0;

        if(load_size > 0) {
            size_t data_read = {};
            amount_of_payloads = load_size / CFG_PAYLOAD_SIZE;

            if(load_size > CFG_PAYLOAD_SIZE)
                if(load_size % CFG_PAYLOAD_SIZE)
                    amount_of_payloads++;

            for(int i = 0; i < (amount_of_payloads - 1); i++) {
                payload_t tmp;
                memcpy(tmp.header.hash, con->info.hash, CFG_PACKET_HASH_SIZE);
                tmp.header.size = CFG_PAYLOAD_SIZE;
                tmp.header.index = i;
                tmp.header.mf = 0x1;

                memcpy(tmp.payload, &(payload[data_read]), CFG_PAYLOAD_SIZE);
                con->payloads->push_back(tmp);

                data_read += CFG_PAYLOAD_SIZE;
            }

            size_t remaining_data = load_size - data_read;

            payload_t final;
            memcpy(final.header.hash, con->info.hash, CFG_PACKET_HASH_SIZE);
            final.header.mf = 0x0;
            final.header.size = remaining_data;
            final.header.index = amount_of_payloads - 1;

            memcpy(final.payload, &(payload[data_read]), remaining_data);
            con->payloads->push_back(final);

            data_read += load_size - data_read;
        }

        con->info.cmd = cmd;
        memcpy(con->info.flags, flags.c_str(), CFG_FLAGS_BUFFER_SIZE);
        con->info.p_count = amount_of_payloads;
        con->info.size = sizeof(packet_t) + (sizeof(payload_t) * con->info.p_count);

        return con;
}

std::string* RFS::retain_payloads(std::vector<payload_t>& pylds) noexcept {
    std::string* payloads = new std::string();
    
    int i;
    for(i = 0; i < pylds.size(); i++)
        for(int j = 0; j < pylds[i].header.size; j++)
            *payloads += pylds[i].payload[j];
    
    return payloads;
}

void RFS::serialize_packet(packet_t& pckt, char* buffer) noexcept {
    // Store size.
    size_t* size_p = (size_t*)buffer;
    *size_p = pckt.size; size_p++;
    // Store p count.
    uint32_t* p_count_p = (uint32_t*)size_p;
    *p_count_p = pckt.p_count; p_count_p++;
    // Store cmd int.
    uint8_t* int_p = (uint8_t*)p_count_p;
    *int_p = pckt.cmd; int_p++;
    // Store hash char.
    char* hash_p = (char*)int_p;
    int hash_c = 0;
    while(hash_c < CFG_PACKET_HASH_SIZE) {
        *hash_p = pckt.hash[hash_c]; 
        hash_p++;
        hash_c++;
    }
    // Store flags char.
    char* char_p = (char*)hash_p;
    int flag_c = 0;

    while(flag_c < CFG_FLAGS_BUFFER_SIZE) {
        *char_p = pckt.flags[flag_c];
        char_p++;
        flag_c++;
    }
}

void RFS::deserialize_packet(packet_t& pckt, char* buffer) noexcept {
    // Store size into struct.
    size_t* size_p = (size_t*)buffer;
    pckt.size = *size_p; size_p++;
    // Store p count.
    uint32_t* p_count_p = (uint32_t*)size_p;
    pckt.p_count = *p_count_p; p_count_p++;
    // Store cmd into struct.
    uint8_t* int_p = (uint8_t*)p_count_p;
    pckt.cmd = *int_p; int_p++;
    // Store hash char.
    char* hash_p = (char*)int_p;
    int hash_c = 0;
    while(hash_c < CFG_PACKET_HASH_SIZE) {
        pckt.hash[hash_c] = *hash_p; 
        hash_p++;
        hash_c++;
    }
    // Store flags into struct.
    char* char_p = (char*)hash_p;
    int flag_c = 0;

    while(flag_c < CFG_FLAGS_BUFFER_SIZE) {
        pckt.flags[flag_c] = *char_p;
        char_p++;
        flag_c++;
    }
}

void RFS::serialize_payload(payload_t& pyld, char* buffer) noexcept {
    // Store payload header.
    payload_header_t* header_p = (payload_header_t*)buffer;
    *header_p = pyld.header; header_p++;
    //Store payload.
    char* char_p = (char*)header_p;
    int pyld_c = 0;

    while(pyld_c < CFG_PAYLOAD_SIZE) {
        *char_p = pyld.payload[pyld_c];
        char_p++;
        pyld_c++;
    }
}

void RFS::deserialize_payload(payload_t& pyld, char* buffer) noexcept {
    // Store payload header.
    payload_header_t* header_p = (payload_header_t*)buffer;
    pyld.header = *header_p; header_p++;
    //Store payload into struct.
    char* char_p = (char*)header_p;
    int pyld_c = 0;

    while(pyld_c < CFG_PAYLOAD_SIZE) {
        pyld.payload[pyld_c] = *char_p;
        char_p++;
        pyld_c++;
    }
}

void RFS::print_packet(const packet_t& pckt) const noexcept {
    BUFFER << "\n---- INFO ----\n -> cmd   : [" << pckt.cmd << "]\n -> flags : [" << pckt.flags << "]\n -> Payload count  : [" << pckt.p_count << "]\n--------------\n";
}

void RFS::print_payload(const payload_t& pyld) const noexcept {
    BUFFER << "\n---- PAYLOAD ----\n -> index : [" << pyld.header.index << "]\n -> size : [" << pyld.header.size << "]\n -> payload : [";

    std::string stream = "";
    for(int i = 0; i < CFG_PAYLOAD_SIZE; i++) {
        stream += pyld.payload[i];
    }
    BUFFER << stream.c_str();
    BUFFER << "]\n-----------------\n";
}