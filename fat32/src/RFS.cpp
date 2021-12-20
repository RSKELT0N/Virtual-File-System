#include "../include/RFS.h"

RFS::pcontainer_t* RFS::generate_container(uint8_t cmd, std::vector<std::string>& args, const char* payload) noexcept {
        pcontainer_t* con = static_cast<pcontainer_t*>(malloc(sizeof(pcontainer_t)));
        packet_t pckt;
        std::vector<payload_t> pyloads;

        std::string flags = {};

        for(const auto& a : args)
            flags += a + " ";

        flags[flags.size() - 1] = '\0';

        pckt.cmd = cmd;
        pckt.flags = flags;

        size_t load_size = strlen(payload);

        if(load_size == 0)
            pckt.ispl = 0x0;
        else pckt.ispl = 0x1;

        if(pckt.ispl == 0x1) {
            size_t data_read = {};
            uint8_t amount_of_payloads = load_size / PAYLOAD_SIZE;

            if(load_size > PAYLOAD_SIZE)
                if(load_size % PAYLOAD_SIZE)
                    amount_of_payloads++;

            for(int i = 0; i < amount_of_payloads - 1; i++) {
                payload_t tmp;
                tmp.mf = 0x1;
                strcpy(tmp.payload, payload + data_read);
                pyloads.push_back(tmp);

                data_read += PAYLOAD_SIZE;
            }

            payload_t final;
            final.mf = 0x0;
            strcpy(final.payload, payload + data_read);
            pyloads.push_back(final);

            data_read += load_size - data_read;
            con->payloads = pyloads;
        }

        con->info = pckt;
        return con;
}