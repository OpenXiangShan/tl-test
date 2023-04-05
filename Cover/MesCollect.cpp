#include "MesCollect.h"

namespace Cover {

    Mes_Collect::Mes_Collect(ScoreBoard<DIR_monitor::Dir_key,DIR_monitor::Dir_Mes> *const selfDir, ScoreBoard<DIR_monitor::Dir_key,paddr_t> *const selfTag
              ,ScoreBoard<DIR_monitor::Dir_key,DIR_monitor::Dir_Mes> *const clientDir, ScoreBoard<DIR_monitor::Dir_key,paddr_t> *const clientTag
                ,Mes_Com *const mc){
        this->Self_Dir_Storage = selfDir;
        this->Self_Dir_Tag_Storage = selfTag;
        this->Client_Dir_Storage = clientDir;
        this->Client_Dir_Tag_Storage = clientTag;
        this->mes_com = mc;
        
    }


//------------------------------TOOL----------------------------------//

    using namespace tl_agent;

    void Mes_Collect::fire_Mes_Collect(const std::shared_ptr<tl_interface::TLInfo> info,uint64_t core_id, uint8_t bus_type){
        
        tl_info = info;

        if(*tl_info->a_valid && *tl_info->a_ready){
            handle_ChnlA_info(core_id,bus_type);
            //for test
            // printf("MES address = %lx\n", Mes.address);
        }
        if(*tl_info->b_valid && *tl_info->b_ready){
            handle_ChnlB_info(core_id,bus_type);
            //for test
            // printf("MES address = %lx\n", Mes.address);
        }
        if(*tl_info->c_valid && *tl_info->c_ready){
            handle_ChnlC_info(core_id,bus_type);
            //for test
            // printf("MES address = %lx\n", Mes.address);
        }
        if(*tl_info->d_valid && *tl_info->d_ready){
            handle_ChnlD_info(core_id,bus_type);
            //for test
            // printf("MES address = %lx\n", Mes.address);
        }
        if(*tl_info->e_valid && *tl_info->e_ready){
            handle_ChnlE_info(core_id,bus_type);
            //for test
            // printf("MES address = %lx\n", Mes.address);
        }

        
    }

//------------------------------GET_CHANNEL_INFO----------------------------------//

    // source in: Acquire* Put* Get
    void Mes_Collect::handle_ChnlA_info(uint64_t core_id, uint8_t bus_type){
        Mes.opcode      =   *tl_info->a_opcode;
        Mes.param       =   *tl_info->a_param;
        Mes.address     =   *tl_info->a_address;
        Mes.bus_type    =   bus_type;
        Mes.core_id     =   core_id;
        Mes.chnl        =   CHNLA;
        Mes.valid       =   true;
        // source
        source[bus_type][core_id][CHNLA].insert(std::make_pair(*tl_info->a_source,*tl_info->a_address));

        // Get State INFO Immediately
        State = get_state_info(Mes.address);
        send(true);
    }
    
    // source in: Probe
    void Mes_Collect::handle_ChnlB_info(uint64_t core_id, uint8_t bus_type){
        Mes.opcode      =   *tl_info->b_opcode;
        Mes.param       =   *tl_info->b_param;
        Mes.address     =   *tl_info->b_address;
        Mes.bus_type    =   bus_type;
        Mes.core_id     =   core_id;
        Mes.chnl        =   CHNLB;
        Mes.valid       =   true;
        // source
        source[bus_type][core_id][CHNLB].insert(std::make_pair(*tl_info->b_source,*tl_info->b_address));

        // Get State INFO Immediately
        State = get_state_info(Mes.address);
        send(true);
    }

    // source in: ReleaseData
    // source erase: ProbeAck*
    void Mes_Collect::handle_ChnlC_info(uint64_t core_id, uint8_t bus_type){
        Mes.opcode      =   *tl_info->c_opcode;
        Mes.param       =   *tl_info->c_param;
        Mes.address     =   *tl_info->c_address;
        Mes.bus_type    =   bus_type;
        Mes.core_id     =   core_id;
        Mes.chnl        =   CHNLC;
        Mes.valid       =   true;
        // source
        if(Mes.opcode == ReleaseData)
            source[bus_type][core_id][CHNLC].insert(std::make_pair(*tl_info->c_source,*tl_info->c_address));
        else if(Mes.opcode == ProbeAck || Mes.opcode == ProbeAckData)
            source[bus_type][core_id][CHNLB].erase(*tl_info->c_source);
        else
            tlc_assert(false,"Illegal opcode!");

        // Get State INFO Immediately
        if(Mes.opcode == ReleaseData){
            State = get_state_info(Mes.address);
            send(true);
        }
        // Probe is an intermediate message, no need to check state
        else if(Mes.opcode == ProbeAck || Mes.opcode == ProbeAckData){
            send(false);
        }else{
            tlc_assert(false,"Illegal opcode!");
        }
    }

    // source erase 1: ReleaseAck
    // source erase 2: Grant*
    // source erase 3: AccessAck*
    // sink in: Grant*
    void Mes_Collect::handle_ChnlD_info(uint64_t core_id, uint8_t bus_type){
        Mes.opcode      =   *tl_info->d_opcode;
        Mes.param       =   *tl_info->d_param;
        Mes.bus_type    =   bus_type;
        Mes.core_id     =   core_id;
        Mes.chnl        =   CHNLD;
        Mes.valid       =   true;
        //sink
        if(Mes.opcode == Grant || Mes.opcode == GrantData)
            sink[bus_type][core_id].insert(std::make_pair(*tl_info->d_sink,
                    source[bus_type][core_id][CHNLA][*tl_info->d_source]));
        //source
        if(Mes.opcode == ReleaseAck || Mes.opcode == ReleaseData){
            Mes.address = source[bus_type][core_id][CHNLC][*tl_info->d_source];
            source[bus_type][core_id][CHNLC].erase(*tl_info->d_source);
        }
        else if(Mes.opcode == Grant || Mes.opcode == GrantData){
            Mes.address = source[bus_type][core_id][CHNLA][*tl_info->d_source];
            source[bus_type][core_id][CHNLA].erase(*tl_info->d_source);
        }
        else if(Mes.opcode == AccessAck || Mes.opcode == AccessAckData){
            Mes.address = source[bus_type][core_id][CHNLA][*tl_info->d_source];
            source[bus_type][core_id][CHNLA].erase(*tl_info->d_source);
        }
        else
            tlc_assert(false,"Illegal opcode!");

        // Waiting for State to be written
        // no need to wait ReleaseAck because DIR write before ReleaseAck
        if(Mes.opcode == AccessAck || Mes.opcode == AccessAckData){
            if(Mes.bus_type == PTW_BUS_TYPE || Mes.bus_type == DMA_BUS_TYPE){
                State = get_state_info(Mes.address);
                send(true);
            }else if(Mes.bus_type != ICACHE_BUS_TYPE && Mes.bus_type != DCACHE_BUS_TYPE){
                send(false);
            }else{
                printf("CHNL:%d BUSTPYE:%d", Mes.chnl,Mes.bus_type);
                tlc_assert(false,"Illegal BUSTYPE!");
            }        
        }else if(Mes.opcode == ReleaseAck){
            State = get_state_info(Mes.address);
            send(true);
        }else if(Mes.opcode == Grant || Mes.opcode == GrantData){
            send(false);
        }else{
            tlc_assert(false,"Illegal opcode!");
        }

    }

    void Mes_Collect::handle_ChnlE_info(uint64_t core_id, uint8_t bus_type){
        Mes.opcode      =   GrantAck;
        Mes.param       =   0;
        Mes.bus_type    =   bus_type;
        Mes.core_id     =   core_id;
        Mes.chnl        =   CHNLE;
        Mes.valid       =   true;
        Mes.address     =   sink[bus_type][core_id][*tl_info->e_sink];
        sink[bus_type][core_id].erase(*tl_info->e_sink);

        // Waiting for State to be written
        if(Mes.bus_type == DCACHE_BUS_TYPE || Mes.bus_type == ICACHE_BUS_TYPE)//L1-L2
            pool.add_wating(Mes.address, core_id, Mes);
        else if(Mes.bus_type == TILE_BUS_TYPE)//L2-L3
            pool.add_wating(Mes.address, ID_L3, Mes);
        else if(Mes.bus_type == L3_BUS_TYPE)//L3-MEM
            send(false);
        else{
            printf("CHNL:%d BUSTPYE:%d", Mes.chnl,Mes.bus_type);
            tlc_assert(false,"Illegal BUSTYPE!");
        }
    }


//------------------------------GET_STATE_INFO----------------------------------//
    using namespace DIR_monitor;
    using namespace Tool;

    cacheState Mes_Collect::get_state_info(paddr_t addr){
                
        // Init
        cacheState State;
        State.L1[ID_CORE0][DCACHE_BUS_TYPE]    =   INVALID;
        State.L1[ID_CORE0][ICACHE_BUS_TYPE]    =   INVALID;
        State.L1[ID_CORE1][DCACHE_BUS_TYPE]    =   INVALID;
        State.L1[ID_CORE1][ICACHE_BUS_TYPE]    =   INVALID;
        State.L2[ID_CORE0]                     =   INVALID;
        State.L2[ID_CORE1]                     =   INVALID;
        State.L3                               =   INVALID;

        //---------collect L1 Cache State---------//
        // Addr to Key
        paddr_t L1_tag;
        Dir_key L1_key;
        L1_tag          =   Get_n_bit(addr, L2_bit[client_tag_index], 64);
        L1_key.set      =   Get_n_bit(addr, L2_bit[set_index], L2_bit[client_tag_index]-1);
        L1_key.slice    =   Get_n_bit(addr, L2_bit[slice_index], L2_bit[set_index]-1);
        // query
        for(uint8_t mod = 0; mod <= 1; mod++){//Search for core0/1 L2
            for (uint8_t i = 0; i < N_WAY; i++)//Search for 0-7 way
            {
                L1_key.way = i;
                if(Client_Dir_Tag_Storage[mod].haskey(L1_key)){
                    // printf("[%d] [%lx] [%lx] [%lx] [%x]\n", mod, L1_tag, L1_key.set, L1_key.slice, L1_key.way);
                    // printf("%d Has Key\n",mod);
                    if(*Client_Dir_Tag_Storage[mod].query(L1_key) == L1_tag){
                        // printf("Tag Ture\n");
                        State.L1[mod][DCACHE_BUS_TYPE] = Client_Dir_Storage[mod].query(L1_key)->client[DCACHE_BUS_TYPE];
                        State.L1[mod][ICACHE_BUS_TYPE] = Client_Dir_Storage[mod].query(L1_key)->client[ICACHE_BUS_TYPE];
                        // printf("D:[%s] I:[%s]\n", stateTostring(State.L1[mod][DCACHE_BUS_TYPE]).c_str(), stateTostring(State.L1[mod][ICACHE_BUS_TYPE]).c_str());
                    }
                }
            }
        }

        //---------collect L2 Cache State---------//
        // Addr to Key
        paddr_t L2_tag;
        Dir_key L2_key;
        L2_tag          =   Get_n_bit(addr, L2_bit[self_tag_index], 64);
        L2_key.set      =   Get_n_bit(addr, L2_bit[set_index], L2_bit[self_tag_index]-1);
        L2_key.slice    =   Get_n_bit(addr, L2_bit[slice_index], L2_bit[set_index]-1);
        // query
        for(uint8_t mod = 0; mod <= 1; mod++){//Search for core0/1 L2
            for (uint8_t i = 0; i < N_WAY; i++)//Search for 0-7 way
            {
                L2_key.way = i;
                if(Self_Dir_Tag_Storage[mod].haskey(L2_key)){
                    // printf("[%d] [%lx] [%lx] [%lx] [%x]\n", mod, L2_tag, L2_key.set, L2_key.slice, L2_key.way);
                    // printf("%d Has Key\n",mod);
                    if(*Self_Dir_Tag_Storage[mod].query(L2_key) == L2_tag){
                        // printf("Tag Ture\n");
                        State.L2[mod] = Self_Dir_Storage[mod].query(L2_key)->self;
                        // printf("L2:[%s]\n", stateTostring(State.L2[mod]).c_str());
                    }
                }
            }
        }

        //---------collect L3 Cache State---------//
        // Addr to Key
        paddr_t L3_tag;
        Dir_key L3_key;
        L3_tag          =   Get_n_bit(addr, L3_bit[self_tag_index], 64);
        L3_key.set      =   Get_n_bit(addr, L3_bit[set_index], L3_bit[self_tag_index]-1);
        L3_key.slice    =   Get_n_bit(addr, L3_bit[slice_index], L3_bit[set_index]-1);
        // query
        uint8_t mod = 2;//Search for L3
        for (uint8_t i = 0; i < N_WAY; i++)//Search for 0-7 way
        {
            L3_key.way = i;
            // printf("Search for l3 %d\n", L3_key.way);
            if(Self_Dir_Tag_Storage[mod].haskey(L3_key)){
                // printf("[%d] [%lx] [%lx] [%lx] [%x]\n", mod, L3_tag, L3_key.set, L3_key.slice, L3_key.way);
                // printf("%d Has Key\n",mod);
                if(*Self_Dir_Tag_Storage[mod].query(L3_key) == L3_tag){
                    // printf("Tag Ture\n");
                    State.L3 = Self_Dir_Storage[mod].query(L3_key)->self;
                    // printf("L3:[%s]\n", stateTostring(State.L3).c_str());
                }
            }
        } 

        return State;
    }


//----------------------------POOL--------------------------------//

    void Mes_Collect::check_time_out(void){
        tlMes mes;
        do{
            mes = pool.check_time();
            if(mes.address != 0x0){
                Mes = mes;
                // printf("check_time_out get_state_info\n");
                State = get_state_info(Mes.address);
                send(true);
            }
        }while(mes.address != 0x0);

        pool.update_time();
    }

    void Mes_Collect::update_pool(paddr_t addr, uint64_t dir_id, bool DIR, uint8_t DirOrTag){

        if(addr == 0x0)
            return;

        uint64_t id;
        if(dir_id < NR_DIR_L2_core0_MONITOR){
            id = ID_CORE0; 
        }else if(dir_id < NR_DIR_L2_core0_MONITOR+NR_DIR_L2_core1_MONITOR){
            id = ID_CORE1;
        }else if(dir_id < NR_DIR_L2_core0_MONITOR+NR_DIR_L2_core1_MONITOR+NR_DIR_L3_MONITOR){
            id = ID_L3;
        }
        // check self
        if(DIR == SELF && pool.self_haskey(addr, id)){
            pool.self_earse_DirOrTag(DirOrTag, addr, id);

            // check client && check tag and dir be write 
            if(!pool.client_haskey(addr, id) && pool.self_be_write_finish(addr, id)){
                // printf("SELF update_pool get_state_info\n");
                // printf("ADDR=%lx\n",addr);
                Mes = pool.get_self(addr, id);
                State = get_state_info(Mes.address);
                send(true);
            }

            if(pool.self_be_write_finish(addr, id))
                pool.erase_self_wating(addr, id);
        }
        // check client && check tag and dir be write 
        else if(DIR == CLIENT && pool.client_haskey(addr, id)){
            pool.client_earse_DirOrTag(DirOrTag, addr, id);
            //check self
            if(!pool.self_haskey(addr, id) && pool.client_be_write_finish(addr, id)){
                // printf("CLIENT update_pool get_state_info\n");
                Mes = pool.get_client(addr, id);
                State = get_state_info(Mes.address);
                send(true);
            }

            if(pool.client_be_write_finish(addr, id))
                pool.erase_client_wating(addr, id);
        }
            
    }

//---------------------------send-------------------------------------//

    bool req_has_data(uint8_t chnl, uint8_t opcode){
        int mes_type;
        switch(chnl){
            case CHNLA: mes_type = REQ; break;
            case CHNLB: mes_type = REQ; break;
            case CHNLC:
                if(opcode == ReleaseData)
                    mes_type = REQ;
                else if(opcode == ProbeAck || opcode == ProbeAckData)
                    mes_type = ACK;
                else
                    tlc_assert(false,"Illegal opcode!");
                break;
            case CHNLD: mes_type = ACK; break;
            case CHNLE: mes_type = ACK1; break;
            default: tlc_assert(false,"Illegal Channel!"); break;
        }
        if(mes_type == REQ){
            if(opcode == PutFullData || opcode == PutPartialData || opcode == ReleaseData)
                return true;
        }
        return false;
    }

    void Mes_Collect::send(bool state_valid){
        package pk;
        pk.mes = Mes;
        pk.state = State;
        pk.state.valid = state_valid;

        // mes has data will be send twice, last send cancel
        static std::set<tlMes> log;
        if(req_has_data(Mes.chnl, Mes.opcode)){
            if(log.count(Mes) > 0){
                log.erase(Mes);
                return;
            }else{
                log.insert(Mes);
            }
        }
        
        printf("SEND SUCCESS! opcode = %d addr = %lx valid: %d\n", pk.mes.opcode, pk.mes.address, pk.state.valid);
        mes_com->arbiter(pk);
    }


} // namespace Cover