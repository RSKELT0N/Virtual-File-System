#include "../include/RFS.h"

char* generate_hash() noexcept {
    char* hash = (char*)malloc(sizeof(char) * CFG_PACKET_HASH_SIZE);


    for(int i = 0; i < CFG_PACKET_HASH_SIZE; i++) {
        int r = rand() % 122 + 36;
        hash[i] = (char)r;
    }
    return hash;
}

RFS::pcontainer_t* RFS::generate_container(uint8_t cmd, std::vector<std::string>& args, char*& payload) noexcept {
        pcontainer_t* con = new pcontainer_t;
        char* hash = generate_hash();
        memcpy(con->info.hash, hash, CFG_PACKET_HASH_SIZE);
        free(hash);

        std::string flags = {};

        for(const auto& a : args)
            flags += a + " ";

        flags[flags.size() - 1] = '\0';
        
        uint64_t load_size = strlen(payload);
        uint64_t amount_of_payloads = 0;

        if(load_size > 0) {
            size_t data_read = {};
            amount_of_payloads = load_size / CFG_PAYLOAD_SIZE;

            if(load_size > CFG_PAYLOAD_SIZE)
                if(load_size % CFG_PAYLOAD_SIZE)
                    amount_of_payloads++;

            if(load_size > 0 && load_size < CFG_PAYLOAD_SIZE)
                amount_of_payloads += 1;
    
            for(int i = 0; (uint64_t)i < (amount_of_payloads - 1); i++) {
                payload_t tmp;
                memcpy(tmp.header.hash, con->info.hash, CFG_PACKET_HASH_SIZE);
                tmp.header.size = CFG_PAYLOAD_SIZE;
                tmp.header.index = i;
                tmp.header.mf = 0x1;

                memcpy(tmp.payload, &(payload[data_read]), CFG_PAYLOAD_SIZE);
                con->payloads->push_back(tmp);

                data_read += CFG_PAYLOAD_SIZE;
            }

            uint64_t remaining_data = load_size - data_read;

            payload_t final;
            memcpy(final.header.hash, con->info.hash, CFG_PACKET_HASH_SIZE);
            final.header.mf = 0x0;
            final.header.size = remaining_data;
            final.header.index = amount_of_payloads - 1;

            memcpy(final.payload, &(payload[data_read]), remaining_data);
            con->payloads->push_back(final);

            data_read += load_size - data_read;
        }
        memcpy(con->info.signature, CFG_PACKET_SIGNATURE, CFG_PACKET_SIGNATURE_SIZE);
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
    // Store signature.
    char* sign_p = (char*)buffer;
    int sign_c = 0;

    while(sign_c < CFG_PACKET_SIGNATURE_SIZE) {
        *sign_p = pckt.signature[sign_c];
        sign_p++;
        sign_c++;
    }
    // Store size.
    uint64_t* size_p = (uint64_t*)sign_p;
    *size_p = pckt.size; size_p++;
    // Store p count.
    uint64_t* p_count_p = (uint64_t*)size_p;
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
    // Store signature.
    char* sign_p = (char*)buffer;
    int sign_c = 0;

    while(sign_c < CFG_PACKET_SIGNATURE_SIZE) {
        pckt.signature[sign_c] = *sign_p;
        sign_p++;
        sign_c++;
    }
    // Store size into struct.
    uint64_t* size_p = (uint64_t*)sign_p;
    pckt.size = *size_p; size_p++;
    // Store p count.
    uint64_t* p_count_p = (uint64_t*)size_p;
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

    while(pyld_c < pyld.header.size) {
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

    while(pyld_c < pyld.header.size) {
        pyld.payload[pyld_c] = *char_p;
        char_p++;
        pyld_c++;
    }
}

uint8_t RFS::process_payload(const packet_t& pckt, const payload_t& pyld) const noexcept {
    uint8_t return_val = 1;

    if(strncmp(pyld.header.hash, pckt.hash, CFG_PACKET_HASH_SIZE) != 0)
        return_val = 0;

    if(pyld.header.size > pckt.size)
        return_val = 0;

    if(pyld.header.index > pckt.p_count)
        return_val = 0;

    
    return return_val;
}

uint8_t RFS::process_packet(const packet_t& pckt) const noexcept {
    uint8_t return_val = 1;

    if(strncmp(pckt.signature, CFG_PACKET_SIGNATURE, CFG_PACKET_SIGNATURE_SIZE) != 0)
        return_val = 0;
    
    if(pckt.size < (pckt.p_count * sizeof(payload_t)))
        return_val = 0;
    
    if(pckt.p_count > (pckt.size / sizeof(payload_t)))
        return_val = 0;

    return return_val;
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