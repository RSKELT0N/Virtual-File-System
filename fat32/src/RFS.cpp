#include "../include/RFS.h"

RFS::pcontainer_t* RFS::generate_container(uint8_t cmd, std::vector<std::string>& args, const char* payload) noexcept {
        pcontainer_t* con = new pcontainer_t;

        std::string flags = {};

        for(const auto& a : args)
            flags += a + " ";

        flags[flags.size() - 1] = '\0';

        con->info.cmd = cmd;
        memcpy(con->info.flags, flags.c_str(), FLAGS_BUFFER_SIZE);

        size_t load_size = strlen(payload);

        if(load_size == 0)
            con->info.ispl = 0x0;
        else con->info.ispl = 0x1;

        if(con->info.ispl == 0x1) {
            size_t data_read = {};
            size_t amount_of_payloads = load_size / PAYLOAD_SIZE;

            if(load_size > PAYLOAD_SIZE)
                if(load_size % PAYLOAD_SIZE)
                    amount_of_payloads++;

            for(int i = 0; i < amount_of_payloads - 1; i++) {
                payload_t tmp;
                tmp.mf = 0x1;
                memcpy(tmp.payload, &(payload[data_read]), PAYLOAD_SIZE);
                con->payloads->push_back(tmp);

                data_read += PAYLOAD_SIZE;
            }

            payload_t final;
            final.mf = 0x0;
            memcpy(final.payload, &(payload[data_read]), load_size - data_read);
            con->payloads->push_back(final);

            data_read += load_size - data_read;
        }
        return con;
}

std::string RFS::retain_payloads(std::vector<payload_t>& pylds) noexcept {
    std::string payloads = {};

    for(int i = 0; i < pylds.size(); i++)
        for(int j = 0; j < PAYLOAD_SIZE; j++)
            payloads += pylds[i].payload[j];
    
    return payloads;
}

void RFS::serialize_packet(packet_t& pckt, char* buffer) noexcept {
    // Store cmd int.
    uint8_t* int_p = (uint8_t*)buffer;
    *int_p = pckt.cmd; int_p++;
    // Store flags char.
    char* char_p = (char*)int_p;
    int flag_c = 0;

    while(flag_c < FLAGS_BUFFER_SIZE) {
        *char_p = pckt.flags[flag_c];
        char_p++;
        flag_c++;
    }
    // Store ispl.
    int_p = (uint8_t*)char_p;
    *int_p = pckt.ispl;
}

void RFS::deserialize_packet(packet_t& pckt, char* buffer) noexcept {
    // Store cmd into struct.
    uint8_t* int_p = (uint8_t*)buffer;
    pckt.cmd = *int_p; int_p++;
    // Store flags into struct.
    char* char_p = (char*)int_p;
    int flag_c = 0;

    while(flag_c < FLAGS_BUFFER_SIZE) {
        pckt.flags[flag_c] = *char_p;
        char_p++;
        flag_c++;
    }
    // Store ispl.
    int_p = (uint8_t*)char_p;
    pckt.ispl = *int_p;
}

void RFS::serialize_payload(payload_t& pyld, char* buffer) noexcept {
    // Store mf int.
    uint8_t* int_p = (uint8_t*)buffer;
    *int_p = pyld.mf; int_p++;
    //Store payload.
    char* char_p = (char*)int_p;
    int pylad_c = 0;

    while(pylad_c < PAYLOAD_SIZE) {
        *char_p = pyld.payload[pylad_c];
        char_p++;
        pylad_c++;
    }
}

void RFS::deserialize_payload(payload_t& pyld, char* buffer) noexcept {
    // Store mf into struct.
    uint8_t* int_p = (uint8_t*)buffer;
    pyld.mf = *int_p; int_p++;
    //Store payload into struct.
    char* char_p = (char*)int_p;
    int pylad_c = 0;

    while(pylad_c < PAYLOAD_SIZE) {
        pyld.payload[pylad_c] = *char_p;
        char_p++;
        pylad_c++;
    }
}

void RFS::print_packet(const packet_t& pckt) const noexcept {
    printf("\n---- INFO ----\n -> cmd   : [%d]\n -> flags : [%s]\n -> ispl  : [%d]\n--------------\n", pckt.cmd, pckt.flags, pckt.ispl);
}

void RFS::print_payload(const payload_t& pyld) const noexcept {
    printf("\n---- PAYLOAD ----\n -> mf : [%d]\n -> payload : [", pyld.mf);

    for(int i = 0; i < PAYLOAD_SIZE; i++) {
        printf("%c", pyld.payload[i]);
    }
    printf("]\n-----------------\n");
}