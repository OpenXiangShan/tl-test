#include "DIR_Write.h"

namespace dir_write{

    DIR_Write::DIR_Write(uint64_t iid, uint8_t bt):
    id(iid),bus_type(bt){
        using namespace tl_interface;

        this->info.reset(new tl_interface::DIRInfo(id, bus_type));
        tl_interface::register_dir_write_info(this->info);
    }

    using namespace tl_agent;

    void DIR_Write::test(){
        paddr_t self_tag = 0x4000;
        paddr_t client_tag = 0x10000;
        paddr_t set = 0x0;
        uint8_t way = 0x0;

        uint8_t self = TIP;
        uint8_t client0 = INVALID;
        uint8_t client1 = INVALID;

        this->info->arbiter.reset(new uint8_t(DIR_WRITE));

        // this->info->dirWReq_ready
        this->info->dirWReq_valid.reset(new uint8_t(1));
        this->info->dirWReq_bits_set.reset(new uint64_t(set));
        this->info->dirWReq_bits_way.reset(new uint8_t(way));
        this->info->dirWReq_bits_data_state.reset(new uint8_t(self));
        this->info->dirWReq_bits_data_clientStates_0.reset(new uint8_t(client0));
        this->info->dirWReq_bits_data_clientStates_1.reset(new uint8_t(client1));

        // this->info->tagWReq_ready
        this->info->tagWReq_valid.reset(new uint8_t(1));
        this->info->tagWReq_bits_set.reset(new uint64_t(set));
        this->info->tagWReq_bits_way.reset(new uint8_t(set));
        this->info->tagWReq_bits_tag.reset(new uint64_t(self_tag));
        
        // this->info->clientDirWReq_ready
        this->info->clientDirWReq_valid.reset(new uint8_t(1));
        this->info->clientDirWReq_bits_set.reset(new uint64_t(set));
        this->info->clientDirWReq_bits_way.reset(new uint8_t(way));
        this->info->clientDirWReq_bits_data_0_state.reset(new uint8_t(client0));
        this->info->clientDirWReq_bits_data_1_state.reset(new uint8_t(client1));

        // this->info->clientTagWreq_ready
        this->info->clientTagWreq_valid.reset(new uint8_t(1));
        this->info->clientTagWreq_bits_set.reset(new uint64_t(set));
        this->info->clientTagWreq_bits_way.reset(new uint8_t(way));
        this->info->clientTagWreq_bits_tag.reset(new uint64_t(client_tag));
    }
}