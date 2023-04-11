#include "DIR_Write.h"

namespace dir_write{

    DIR_Write::DIR_Write(uint8_t bt):
    bus_type(bt){
        using namespace tl_interface;
        for (uint64_t i = 0; i < MAX; i++)
        {
            this->info[i].reset(new tl_interface::DIRInfo(i, bus_type));
            tl_interface::register_dir_write_info(this->info[i]);
        }
    }

    using namespace tl_agent;

    void DIR_Write::output(uint64_t id, paddr_t self_tag, paddr_t client_tag, paddr_t set, uint8_t way, uint8_t self, uint8_t client0, uint8_t client1){

        this->info[id]->arbiter.reset(new uint8_t(DIR_WRITE));
        
        
        if(self != INVALID){
            // this->info[id]->dirWReq_ready
            this->info[id]->dirWReq_valid.reset(new uint8_t(1));
            this->info[id]->dirWReq_bits_set.reset(new uint64_t(set));
            this->info[id]->dirWReq_bits_way.reset(new uint8_t(way));
            this->info[id]->dirWReq_bits_data_state.reset(new uint8_t(self));
            this->info[id]->dirWReq_bits_data_clientStates_0.reset(new uint8_t(client0));
            this->info[id]->dirWReq_bits_data_clientStates_1.reset(new uint8_t(client1));

            // this->info[id]->tagWReq_ready
            this->info[id]->tagWReq_valid.reset(new uint8_t(1));
            this->info[id]->tagWReq_bits_set.reset(new uint64_t(set));
            this->info[id]->tagWReq_bits_way.reset(new uint8_t(set));
            this->info[id]->tagWReq_bits_tag.reset(new uint64_t(self_tag));
        }else{
            this->info[id]->dirWReq_valid.reset(new uint8_t(0));//valid
            this->info[id]->dirWReq_bits_set.reset(new paddr_t(0));//addr
            this->info[id]->dirWReq_bits_way.reset(new uint8_t(0));//addr
            this->info[id]->dirWReq_bits_data_state.reset(new uint8_t(0));
            this->info[id]->dirWReq_bits_data_clientStates_0.reset(new uint8_t(0));
            this->info[id]->dirWReq_bits_data_clientStates_1.reset(new uint8_t(0));

            this->info[id]->tagWReq_valid.reset(new uint8_t(0));//valid
            this->info[id]->tagWReq_bits_set.reset(new paddr_t(0));//addr
            this->info[id]->tagWReq_bits_way.reset(new uint8_t(0));//addr
            this->info[id]->tagWReq_bits_tag.reset(new paddr_t(0));//addr
        }
        
        // this->info[id]->clientDirWReq_ready
        this->info[id]->clientDirWReq_valid.reset(new uint8_t(1));
        this->info[id]->clientDirWReq_bits_set.reset(new uint64_t(set));
        this->info[id]->clientDirWReq_bits_way.reset(new uint8_t(way));
        this->info[id]->clientDirWReq_bits_data_0_state.reset(new uint8_t(client0));
        this->info[id]->clientDirWReq_bits_data_1_state.reset(new uint8_t(client1));

        // this->info[id]->clientTagWreq_ready
        this->info[id]->clientTagWreq_valid.reset(new uint8_t(1));
        this->info[id]->clientTagWreq_bits_set.reset(new uint64_t(set));
        this->info[id]->clientTagWreq_bits_way.reset(new uint8_t(way));
        this->info[id]->clientTagWreq_bits_tag.reset(new uint64_t(client_tag));
    }

    void DIR_Write::write(testcase_with_states::states state, paddr_t addr){
        // Addr to Key
        using namespace Tool;
        using namespace DIR_monitor;
        paddr_t self_tag[MAX];
        paddr_t self_set[MAX];
        uint8_t self_way[MAX] = {0, 0, 0};
        paddr_t client_tag[MAX];
        paddr_t client_set[MAX];
        uint8_t client_way[MAX] = {0, 0, 0};
        // L2 CORE0
        self_tag[0]        =   Get_n_bit(addr, L2_bit[self_tag_index], 64);
        self_set[0]        =   Get_n_bit(addr, L2_bit[set_index], L2_bit[self_tag_index]-1);
        client_tag[0]      =   Get_n_bit(addr, L2_bit[client_tag_index], 64);
        client_set[0]      =   Get_n_bit(addr, L2_bit[set_index], L2_bit[client_tag_index]-1);
        //L2 CORE1
        self_tag[1]        =   Get_n_bit(addr, L2_bit[self_tag_index], 64);
        self_set[1]        =   Get_n_bit(addr, L2_bit[set_index], L2_bit[self_tag_index]-1);
        client_tag[1]      =   Get_n_bit(addr, L2_bit[client_tag_index], 64);
        client_set[1]      =   Get_n_bit(addr, L2_bit[set_index], L2_bit[client_tag_index]-1);
        // L3
        self_tag[2]        =   Get_n_bit(addr, L3_bit[self_tag_index], 64);
        self_set[2]        =   Get_n_bit(addr, L3_bit[set_index], L3_bit[self_tag_index]-1);
        client_tag[2]      =   Get_n_bit(addr, L3_bit[client_tag_index], 64);
        client_set[2]      =   Get_n_bit(addr, L3_bit[set_index], L3_bit[client_tag_index]-1);

        // for test
        // output(L2_0 ,self_tag[0], client_tag[0], self_set[0], self_way[0], TIP, INVALID, INVALID);
        // output(L2_1 ,self_tag[1], client_tag[1], self_set[1], self_way[1], TIP, INVALID, INVALID);
        // output(L3 ,self_tag[2], client_tag[2], self_set[2], self_way[2], TIP, INVALID, INVALID);
        
        for(uint64_t id = 0; id < MAX; id++){
            // check ready
            if(info[id]->dirWReq_ready && info[id]->tagWReq_ready && info[id]->clientDirWReq_ready && info[id]->clientTagWreq_ready){
                output(id ,self_tag[id], client_tag[id], self_set[id], self_way[id], state.self[id], state.client[id][0], state.client[id][1]);
            }else{
                tlc_assert(false, "DIR WRITE ready error!\n");
            }
                
        }
    }
}