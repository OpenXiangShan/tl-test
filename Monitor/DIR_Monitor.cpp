#include "DIR_Monitor.h"

namespace DIR_monitor{
    //DIR
    DIR_Monitor::DIR_Monitor(ScoreBoard<Dir_key,Dir_Mes> *const selfDir, ScoreBoard<Dir_key,paddr_t> *const selfTag
                            ,ScoreBoard<Dir_key,Dir_Mes> *const clientDir, ScoreBoard<Dir_key,paddr_t> *const clientTag
                            ,uint64_t* c, uint64_t iid, uint8_t bt):
        cycle(c),id(iid), bus_type(bt){
        this->Self_Dir_Storage = selfDir;
        this->Self_Dir_Tag_Storage = selfTag;
        this->Client_Dir_Storage = clientDir;
        this->Client_Dir_Tag_Storage = clientTag;
        this->info.reset(new tl_interface::DIRInfo(iid, bt));
        tl_interface::register_dir_monitor_info(this->info);
    }

    std::shared_ptr<DIRInfo> DIR_Monitor::get_info(){
        return this->info;
    }

    void DIR_Monitor::print_info(){
        if(*this->info->id < NR_DIR_L2_core0_MONITOR){
            fire_Self_DIR(0,id,L2_bit);//0: core0; id: 0-3
            fire_Client_DIR(0,id,L2_bit);//0: core0; id: 0-3
        }else if(*this->info->id < NR_DIR_L2_core0_MONITOR+NR_DIR_L2_core1_MONITOR){
            fire_Self_DIR(1,id-NR_DIR_L2_core0_MONITOR,L2_bit);//1: core1; id: 0-3
            fire_Client_DIR(1,id-NR_DIR_L2_core0_MONITOR,L2_bit);//1: core1; id: 0-3
        }else if(*this->info->id < NR_DIR_MONITOR){
            fire_Self_DIR(2,id-NR_DIR_L2_core0_MONITOR-NR_DIR_L2_core1_MONITOR,L3_bit);//2: L3; id: 0-3
            fire_Client_DIR(2,id-NR_DIR_L2_core0_MONITOR-NR_DIR_L2_core1_MONITOR,L3_bit);//2: L3; id: 0-3
        }else{
            printf("DIR info input error\n");
        }
        
    }

//------------------------------------Self---------------------------------------------//
    using namespace Tool;

    void DIR_Monitor::fire_Self_DIR(uint8_t mod, paddr_t slice, uint8_t bit[4]){
        //INIT
        std::shared_ptr<DIRInfo> fire_Info = info;
        self_write = false;
        self_write_1 = false;
        
        //self_dir_tag
        if(*fire_Info->tagWReq_valid&&*fire_Info->tagWReq_ready){
            //key
            Dir_key key;
            key.set = *fire_Info->tagWReq_bits_set;
            key.slice = slice;
            key.way = *fire_Info->tagWReq_bits_way;

            //storage
            //TODO *fire_Info->clientTagWreq_bits_tag = a00001000
            //but it should be 1000, tag value out of bounds
            std::shared_ptr<paddr_t> tag;
            tag.reset(new paddr_t);
            *tag = Get_n_bit(*fire_Info->tagWReq_bits_tag,0,19);
            Self_Dir_Tag_Storage[mod].update(key,tag);

            //print
            print_Self_DIR_TAG(mod,key);

            //check
            bool check_finish;
            check_finish = Self_tag_check_pool.erase_check(key);

            // flag for Mes_Collect
            if(check_finish){
                self_write_1 = true;
                self_write_addr_1 = (*tag << bit[self_tag_index]) + (key.set << bit[set_index]) + (key.slice << bit[slice_index]);
            }
        }

        //self_dir_state
        if(*fire_Info->dirWReq_ready&&*fire_Info->dirWReq_valid){
            //key
            Dir_key key;
            key.set = *fire_Info->dirWReq_bits_set;
            key.slice = slice;
            key.way = *fire_Info->dirWReq_bits_way;

            //state
            Dir_Mes state;
            state.self = *fire_Info->dirWReq_bits_data_state;
            state.client[0] = *fire_Info->dirWReq_bits_data_clientStates_0;
            state.client[1] = *fire_Info->dirWReq_bits_data_clientStates_1;
            std::shared_ptr<Dir_Mes> data;
            data.reset(new Dir_Mes(state));

            // storage
            Self_Dir_Storage[mod].update(key, data);

            //print
            print_Self_DIR(mod,key);

            //check self write but tag no write
            //if self state == INVALID,self client state and tag useless, no need to change tag
            if(!Self_Dir_Tag_Storage[mod].haskey(key) && state.self != INVALID){
                printf("check: %s\n", print.add_tag(true).c_str());
                Self_tag_check_pool.add_check(key);
            }else if(Self_Dir_Tag_Storage[mod].haskey(key)){//print addr
                paddr_t addr = (*Self_Dir_Tag_Storage[mod].query(key) << bit[self_tag_index])
                                    + (key.set << bit[set_index])
                                    + (key.slice << bit[slice_index]);
                printf("self dir addr: %s\n", hex_to_str(addr,8,false).c_str());

                // flag for Mes_Collect
                self_write = true;
                self_write_addr = addr;
            }

            
        }

        //check
        Self_tag_check_pool.check_time_out();
    }

    void DIR_Monitor::print_Self_DIR(uint8_t mod, Dir_key key){
        //reset
        print.all = "";

        //title
        print.all = print.all + print.add_title(mod,*cycle);

        //state
        Dir_Mes self_mes = *Self_Dir_Storage[mod].query(key);
        print.self_state = stateTostring(self_mes.self);
        print.selfclient0 = stateTostring(self_mes.client[0]);
        print.selfclient1 = stateTostring(self_mes.client[1]);

        //set slice way
        print.set =  hex_to_str(key.set,2,false);
        print.slice =  hex_to_str(key.slice,1,false);
        print.way =  hex_to_str(key.way,1,false);
        
        //print
        print.all = print.all + print.add_self();
        printf("%s\n", print.all.c_str());
    }

    void DIR_Monitor::print_Self_DIR_TAG(uint8_t mod, Dir_key key){
        //reset
        print.all = "";

        //title
        print.all = print.all + print.add_title(mod,*cycle);

        //set slice way
        print.set =  hex_to_str(key.set,2,false);
        print.slice =  hex_to_str(key.slice,1,false);
        print.way =  hex_to_str(key.way,1,false);

        //tag
        print.tag = hex_to_str(*Self_Dir_Tag_Storage[mod].query(key),5,false);    

        //print
        print.all = print.all + print.add_tag(SELF);
        printf("%s\n", print.all.c_str());
    }


//------------------------------------Client---------------------------------------------//

    void DIR_Monitor::fire_Client_DIR(uint8_t mod, paddr_t slice, uint8_t bit[4]){
        //INIT
        std::shared_ptr<DIRInfo> fire_Info = info;
        client_write = false;
        client_write_1 = false;
        
        //Client_dir_tag
        if(*fire_Info->clientTagWreq_valid&&*fire_Info->clientTagWreq_ready){
            //key
            Dir_key key;
            key.set = *fire_Info->clientTagWreq_bits_set;
            key.slice = slice;
            key.way = *fire_Info->clientTagWreq_bits_way;

            //storage
            //TODO *fire_Info->clientTagWreq_bits_tag = a00001000
            //but it should be 1000, tag value out of bounds
            std::shared_ptr<paddr_t> tag;
            tag.reset(new paddr_t);
            *tag = Get_n_bit(*fire_Info->clientTagWreq_bits_tag,0,19);
            Client_Dir_Tag_Storage[mod].update(key,tag);

            //print
            print_Client_DIR_TAG(mod,key);

            //check
            bool check_finish;
            check_finish = Client_tag_check_pool.erase_check(key);

            // flag for Mes_Collect
            if(check_finish){
                client_write_1 = true;
                client_write_addr_1 = (*tag << bit[client_tag_index]) + (key.set << bit[set_index]) + (key.slice << bit[slice_index]);
            }
        }

        //client_dir_state
        if(*fire_Info->clientDirWReq_ready&&*fire_Info->clientDirWReq_valid){
            //key
            Dir_key key;
            key.set = *fire_Info->clientDirWReq_bits_set;
            key.slice = slice;
            key.way = *fire_Info->clientDirWReq_bits_way;

            //state
            Dir_Mes state;
            state.client[0] = *fire_Info->clientDirWReq_bits_data_0_state;
            state.client[1] = *fire_Info->clientDirWReq_bits_data_1_state;
            std::shared_ptr<Dir_Mes> data;
            data.reset(new Dir_Mes(state));

            // storage
            Client_Dir_Storage[mod].update(key, data);

            //print
            print_Client_DIR(mod,key);

            // check client write but tag no write
            if(!Client_Dir_Tag_Storage[mod].haskey(key)){
                printf("check: %s\n", print.add_tag(true).c_str());
                Client_tag_check_pool.add_check(key);
            }else if(Client_Dir_Tag_Storage[mod].haskey(key)){//print addr
                paddr_t addr = (*Client_Dir_Tag_Storage[mod].query(key) << bit[client_tag_index])
                                    + (key.set << bit[set_index])
                                    + (key.slice << bit[slice_index]);
                printf("Client dir addr: %s\n", hex_to_str(addr,8,false).c_str());
                client_write = true;
                client_write_addr = addr;
            }
        }

        //check
        Client_tag_check_pool.check_time_out();
    }


    void DIR_Monitor::print_Client_DIR(uint8_t mod, Dir_key key){
        //reset
        print.all = "";

        //title
        print.all = print.all + print.add_title(mod,*cycle);

        //state
        Dir_Mes client_mes = *Client_Dir_Storage[mod].query(key);
        print.client0 = stateTostring(client_mes.client[0]);
        print.client1 = stateTostring(client_mes.client[1]);

        //set slice way
        print.set =  hex_to_str(key.set,2,false);
        print.slice =  hex_to_str(key.slice,1,false);
        print.way =  hex_to_str(key.way,1,false);
        
        //print
        print.all = print.all + print.add_client();
        printf("%s\n", print.all.c_str());
    }

    void DIR_Monitor::print_Client_DIR_TAG(uint8_t mod, Dir_key key){
        //reset
        print.all = "";

        //title
        print.all = print.all + print.add_title(mod,*cycle);

        //set slice way
        print.set =  hex_to_str(key.set,2,false);
        print.slice =  hex_to_str(key.slice,1,false);
        print.way =  hex_to_str(key.way,1,false);

        //tag
        print.tag = hex_to_str(*Client_Dir_Tag_Storage[mod].query(key),5,false);    

        //print
        print.all = print.all + print.add_tag(CLIENT);
        printf("%s\n", print.all.c_str());
    }

}