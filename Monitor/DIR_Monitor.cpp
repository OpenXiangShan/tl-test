#include "DIR_Monitor.h"
namespace DIR_monitor{
    //DIR
    DIR_Monitor::DIR_Monitor(uint64_t* c, uint64_t iid, uint8_t bt):
        cycle(c),id(iid), bus_type(bt){
        this->info.reset(new tl_interface::DIRInfo(iid, bt));
        tl_interface::register_dir_monitor_info(this->info);
    }

    paddr_t KeyToAddr(Dir_key key,int a,int b){
        return (key.set<<a)+(key.slice<<b);
    }

    string hex_to_str(uint64_t mask,int len,bool x){
        string hexes[16] = {"0","1","2","3","4","5","6","7","8","9","a","b","c","d","e","f"};
        string hexstring = "";
        for (int i=0; i<len; i++){
        int j = len-i-1;
        int number = (mask >> 4*j) & 0xf;
        hexstring += hexes[number];
        }
        if(x == true)
        return "0x" + hexstring;
        else 
        return hexstring;
    }

    string stateTostring(uint8_t state){
        string s;
        switch (state)
        {
        case TIP:
            s = "TT";
            break;
        case TRUNK:
            s = "T";
            break;
        case BRANCH:
            s = "B";
            break;
        case INVALID:
            s = "N";
            break;
        default: s = "error";
            break;
        }
        return s;
    }

    bool DIR_Monitor::check(uint8_t mod, Dir_key key){
        return true;
    }

//------------------------------------Self---------------------------------------------//

    void DIR_Monitor::fire_Self_DIR(uint8_t mod, paddr_t slice, uint8_t bit[4]){
        std::shared_ptr<DIRInfo> fire_Info = info;
        
        //self_dir_tag
        if(*fire_Info->tagWReq_valid&&*fire_Info->tagWReq_ready){
            //key
            Dir_key key;
            key.set = *fire_Info->tagWReq_bits_set;
            key.slice = slice;
            key.way = *fire_Info->tagWReq_bits_way;

            //storage
            Self_Dir_Tag_Storage[mod].update(key,fire_Info->tagWReq_bits_tag);

            //print
            print_Self_DIR_TAG(mod,key);

            //check
            Self_tag_check_pool->erase_check(key);
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

            //check
            //if self state == INVALID,self client state and tag useless, no need to change tag
            if(!Self_Dir_Tag_Storage[mod].haskey(key) && state.self != INVALID){
                printf("check: %s\n", print.add_tag(true).c_str());
                Self_tag_check_pool->add_check(key);
            }else if(Self_Dir_Tag_Storage[mod].haskey(key)){//print addr
                paddr_t addr = (*Self_Dir_Tag_Storage[mod].query(key) << bit[self_tag_index])
                                    + (key.set << bit[set_index])
                                    + (key.slice << bit[slice_index]);
                printf("self dir addr: %s\n", hex_to_str(addr,8,false).c_str());
            }
        }

        //check
        Self_tag_check_pool->check_time_out();
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
        std::shared_ptr<DIRInfo> fire_Info = info;
        
        //Client_dir_tag
        if(*fire_Info->clientTagWreq_valid&&*fire_Info->clientTagWreq_ready){
            //key
            Dir_key key;
            key.set = *fire_Info->clientTagWreq_bits_set;
            key.slice = slice;
            key.way = *fire_Info->clientTagWreq_bits_way;

            //storage
            Client_Dir_Tag_Storage[mod].update(key,fire_Info->clientTagWreq_bits_tag);

            //print
            print_Client_DIR_TAG(mod,key);

            //check
            Client_tag_check_pool->erase_check(key);
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

            //check
            if(!Client_Dir_Tag_Storage[mod].haskey(key)){
                printf("check: %s\n", print.add_tag(true).c_str());
                Client_tag_check_pool->add_check(key);
            }else if(Client_Dir_Tag_Storage[mod].haskey(key)){//print addr
                paddr_t addr = (*Client_Dir_Tag_Storage[mod].query(key) << bit[client_tag_index])
                                    + (key.set << bit[set_index])
                                    + (key.slice << bit[slice_index]);
                printf("Client dir addr: %s\n", hex_to_str(addr,8,false).c_str());
            }
        }

        //check
        Client_tag_check_pool->check_time_out();
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


    // printf("self_tag tag: %lx self_dir set: %lx slice: %lx self_dir way: %x addr: %lx\n"
    // ,*Self_Dir_Tag_Storage[mod].query(key) << bit[self_tag_index]
    // ,key.set << bit[set_index]
    // ,key.slice << bit[slice_index]
    // ,key.way
    // ,addr);

    // void DIR_Monitor::fire_DIR(uint8_t mod, paddr_t slice,uint8_t bit[4]){
    //     print.all = "";
    //     print.all = print.all + print.add_title();
    //     std::shared_ptr<DIRInfo> fire_Info = info;
    //     //write self_dir
    //     if(*fire_Info->dirWReq_ready&&*fire_Info->dirWReq_valid){
    //         //for test
    //         printf("cycle: %ld core %s %s \n",*cycle,print.core.c_str(), print.mod.c_str());
    //         //state
    //         print.self_state = stateTostring(*fire_Info->dirWReq_bits_data_state);
    //         print.selfclient0 = stateTostring(*fire_Info->dirWReq_bits_data_clientStates_0);
    //         print.selfclient1 = stateTostring(*fire_Info->dirWReq_bits_data_clientStates_1);
    //         //for test
    //         printf("self: %s client0: %s client1: %s\n",print.self_state.c_str(),print.selfclient0.c_str(),print.selfclient1.c_str());
    //         //addr
    //         Dir_key key;
    //         paddr_t addr;
    //         key.set = *fire_Info->tagWReq_bits_set;
    //         key.slice = slice;
    //         key.way = *fire_Info->tagWReq_bits_way;
    //         //for test
    //         if(true){
    //             printf("self_tag tag: %s self_dir set: %ld self_tag set: %ld slice: %ld self_dir way: %d self_tag way: %d self_tag set+tag: %s\n"
    //                 ,hex_to_str(*fire_Info->tagWReq_bits_tag<<bit[self_tag_index],12,true).c_str()
    //                 // ,hex_to_str(*fire_Info->dirWReq_bits_set<<bit[set_index],12,true).c_str()
    //                 ,*fire_Info->dirWReq_bits_set
    //                 ,*fire_Info->tagWReq_bits_set
    //                 ,slice
    //                 ,*fire_Info->dirWReq_bits_way
    //                 ,*fire_Info->tagWReq_bits_way
    //                 ,hex_to_str(KeyToAddr(key,bit[set_index],bit[slice_index]),12,true).c_str());
    //         }

    //         //write self tag
    //         if(*fire_Info->tagWReq_valid&&*fire_Info->tagWReq_ready){

    //             Tag_Storage[mod].update(key, fire_Info->tagWReq_bits_tag);
    //             addr = (*fire_Info->tagWReq_bits_tag<<bit[self_tag_index]) + KeyToAddr(key,bit[set_index],bit[slice_index]);
    //             print.self_dir_addr = hex_to_str(addr,8,false);
    //         }else if(Tag_Storage[mod].haskey(key)){
    //             addr = (*Tag_Storage[mod].query(key)<<bit[self_tag_index]) + KeyToAddr(key,bit[set_index],bit[slice_index]);
    //             print.self_dir_addr = hex_to_str(addr,8,false);
    //         }else if(*fire_Info->dirWReq_bits_data_state == INVALID){//if self state == INVALID,self client state and tag useless, no need to change tag
    //             print.self_dir_addr = "-";
    //         }else{
    //             print.self_dir_addr = "-";
    //         }
    //         print.all = print.all + print.add_self(); 
    //     }

    //     //write self tag alone
    //     if(*fire_Info->tagWReq_valid&&*fire_Info->tagWReq_ready){
    //         //addr
    //         Dir_key key;
    //         paddr_t addr;
    //         key.set = *fire_Info->tagWReq_bits_set;
    //         key.slice = slice;
    //         key.way = *fire_Info->tagWReq_bits_way;
    //         //for test
    //         if(true){
    //             printf("self_tag tag: %s self_dir set: %ld self_tag set: %ld slice: %ld self_dir way: %d self_tag way: %d self_tag set+tag: %s\n"
    //                 ,hex_to_str(*fire_Info->tagWReq_bits_tag<<bit[self_tag_index],12,true).c_str()
    //                 // ,hex_to_str(*fire_Info->dirWReq_bits_set<<bit[set_index],12,true).c_str()
    //                 ,*fire_Info->dirWReq_bits_set
    //                 ,*fire_Info->tagWReq_bits_set
    //                 ,slice
    //                 ,*fire_Info->dirWReq_bits_way
    //                 ,*fire_Info->tagWReq_bits_way
    //                 ,hex_to_str(KeyToAddr(key,bit[set_index],bit[slice_index]),12,true).c_str());
    //         }

    //         Tag_Storage[mod].update(key, fire_Info->tagWReq_bits_tag);
    //         addr = (*fire_Info->tagWReq_bits_tag<<bit[self_tag_index]) + KeyToAddr(key,bit[set_index],bit[slice_index]);
    //         print.self_dir_addr = hex_to_str(addr,8,false);
    //         print.all = print.all + print.add_self_tag_alone(); 
    //     }


        //client_dir
        // if(*fire_Info->clientDirWReq_valid&&*fire_Info->clientDirWReq_ready){
        //     //state
        //     print.client0 = stateTostring(*fire_Info->clientDirWReq_bits_data_0_state);
        //     print.client1 = stateTostring(*fire_Info->clientDirWReq_bits_data_1_state);
        //     //addr
        //     paddr_t client_key;
        //     paddr_t client_addr;
        //     //for test
        //     if(*fire_Info->clientTagWreq_valid&&*fire_Info->clientTagWreq_ready){
        //         printf("cycle: %ld  core%s %s client: tag: %s set: %s way: %s\n",*cycle,print.core.c_str(), print.mod.c_str()
        //             ,hex_to_str(*fire_Info->clientTagWreq_bits_tag<<bit[4],8,false).c_str()
        //             ,hex_to_str(*fire_Info->clientTagWreq_bits_set<<bit[0],8,false).c_str()
        //             ,hex_to_str(*fire_Info->clientTagWreq_bits_way<<bit[2],8,false).c_str());
        //     }
            
        //     client_key = AddrToKey(*fire_Info->clientTagWreq_bits_set,slice,*fire_Info->clientTagWreq_bits_way,8,6,3);
        //     if(*fire_Info->clientTagWreq_valid&&*fire_Info->clientTagWreq_ready){
        //         Client_Tag_Storage[mod].update(client_key, fire_Info->clientTagWreq_bits_tag);
        //         client_addr = (*fire_Info->clientTagWreq_bits_tag<<bit[4]) + client_key;//bit[20:0]  io_clientTagWreq_bits_tag,
        //     }else{
        //         if (!Client_Tag_Storage[mod].haskey(client_key))
        //         {
        //             printf("core%s %s Client Tag no found!key: %s\n",print.core.c_str(), print.mod.c_str(),hex_to_str(client_key,4,false).c_str());
        //         }
        //         client_addr = (*Client_Tag_Storage[mod].query(client_key)<<bit[4]) + client_key;
        //     }
        //     print.client_dir_addr = hex_to_str(client_addr,8,false);
        //     print.all = print.all + print.add_client(); 
        // }

    //     if((*fire_Info->dirWReq_ready&&*fire_Info->dirWReq_valid) 
    //             // || (*fire_Info->clientDirWReq_ready && *fire_Info->clientDirWReq_valid))
    //         )
    //         printf("%s\n", print.all.c_str());
    // }
 

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
        
        // if(*this->info->clientTagWreq_valid){
        // printf("DIR_Monitor running [id] %ld [bus_type] %d\n", *this->info->id, *this->info->bus_type);
        // printf("%lx\n",*this->info->clientTagWreq_bits_tag);
        // printf("%lx\n",*this->info->clientTagWreq_bits_set);
        // printf("%lx\n",test.AddrToKey(0x7a,0x02,0x00));
        // }
        
    }

}