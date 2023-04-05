#include "MesCom.h"

namespace Cover{

using namespace tl_agent;

//---------------------link------------------------//

void link::print(){
    for (auto it = link.begin(); it != link.end(); it++) {
        link_index key = it->first;
        link_col val = it->second;
        printf("REQ: opcode [%d] param [%d] address [%lx]\n", val.Mes_Req.opcode, val.Mes_Req.param, val.Mes_Req.address);
        printf("ACK: opcode [%d] param [%d] address [%lx]\n", val.Mes_Ack.opcode, val.Mes_Ack.param, val.Mes_Ack.address);
        printf("ACK1: opcode [%d] param [%d] address [%lx]\n", val.Mes_Ack1.opcode, val.Mes_Ack1.param, val.Mes_Ack1.address);
        using namespace Tool;
        // Begin
        printf("\n[%s]-[%s]  [%s]-[%s]\n" , stateTostring(val.State_b.L1[ID_CORE0][DCACHE_BUS_TYPE]).c_str() , stateTostring(val.State_b.L1[ID_CORE0][ICACHE_BUS_TYPE]).c_str() 
                                        , stateTostring(val.State_b.L1[ID_CORE1][DCACHE_BUS_TYPE]).c_str() , stateTostring(val.State_b.L1[ID_CORE1][ICACHE_BUS_TYPE]).c_str() );
        printf("  [%s]      [%s]\n"    , stateTostring(val.State_b.L2[ID_CORE0]).c_str(), stateTostring(val.State_b.L2[ID_CORE1]).c_str() );
        printf("       [%s]\n\n"         , stateTostring(val.State_b.L3).c_str() );
        // End
        printf("[%s]-[%s]  [%s]-[%s]\n" , stateTostring(val.State_e.L1[ID_CORE0][DCACHE_BUS_TYPE]).c_str() , stateTostring(val.State_e.L1[ID_CORE0][ICACHE_BUS_TYPE]).c_str() 
                                        , stateTostring(val.State_e.L1[ID_CORE1][DCACHE_BUS_TYPE]).c_str() , stateTostring(val.State_e.L1[ID_CORE1][ICACHE_BUS_TYPE]).c_str() );
        printf("  [%s]      [%s]\n"    , stateTostring(val.State_e.L2[ID_CORE0]).c_str(), stateTostring(val.State_e.L2[ID_CORE1]).c_str() );
        printf("       [%s]\n\n"         , stateTostring(val.State_e.L3).c_str() );
    }
    
}

void link::update_all(package pk){
    tlMes mes = pk.mes;
    cacheState state = pk.state;
    
    // Convert to Message Type
    uint8_t mes_type;
    switch(mes.chnl){
        case CHNLA: mes_type = REQ; break;
        case CHNLB: mes_type = REQ; break;
        case CHNLC:
            if(mes.opcode == ReleaseData)
                mes_type = REQ;
            else if(mes.opcode == ProbeAck || mes.opcode == ProbeAckData)
                mes_type = ACK;
            else
                tlc_assert(false,"Illegal opcode!");
            break;
        case CHNLD: mes_type = ACK; break;
        case CHNLE: mes_type = ACK1; break;
        default: tlc_assert(false,"Illegal Channel!"); break;
    }

    // // Message legitimacy check
    // if(mes_type == REQ)
    //     check_req(mes);

    // insert mes and state to col
    link_index index;
    index.bus_type = mes.bus_type;
    index.core_id = mes.core_id;
    link_col col;
    // storage previous value
    if(link.count(index) > 0)
        col = link[index];
    
    switch (mes_type)
    {
        case REQ: col.Mes_Req = mes; if(pk.state.valid) col.State_b = state; break;
        case ACK: col.Mes_Ack = mes; if(pk.state.valid) col.State_e = state; break;
        case ACK1: col.Mes_Ack1 = mes; if(pk.state.valid) col.State_e = state; break;
        default: tlc_assert(false,"Illegal mes type!"); break;
    }
    // storage first require index
    if(link.empty())
        first_col_index = index;
    // insert
    // link.insert(std::make_pair(index, col));
    link[index] = col;

    // check finish
    check_finish();
}

bool link::updata_first(package pk){
    //--------------Init----------------//
    tlMes mes = pk.mes;
    cacheState state = pk.state;
    link_index index;
    index.bus_type = mes.bus_type;
    index.core_id = mes.core_id;
    link_col col;  
    if(first_in_link){
        reset();
        first_in_link = false;
    }

    
    //------------Convert to Message Type---------//
    uint8_t mes_type;
    switch(mes.chnl){
        case CHNLA: mes_type = REQ; break;
        case CHNLB: mes_type = REQ; break;
        case CHNLC:
            if(mes.opcode == ReleaseData)
                mes_type = REQ;
            else if(mes.opcode == ProbeAck || mes.opcode == ProbeAckData)
                mes_type = ACK;
            else
                tlc_assert(false,"Illegal opcode!");
            break;
        case CHNLD: mes_type = ACK; break;
        case CHNLE: mes_type = ACK1; break;
        default: tlc_assert(false,"Illegal Channel!"); break;
    }


    //---------------Checking input legitimacy----------------//
    // only storage the first message link and
    // Do not allow two modules in the system to request a block at the same time(not a sub-request)
    if(!link.empty() && index != first_col_index && mes_type == REQ){
        if(mes.chnl == CHNLC && mes.opcode == ReleaseData){
            reset();
            return false;
        }else if(mes.chnl == CHNLB && mes.opcode == Probe){
            return false;
        }else{
            if(mes.bus_type == DCACHE_BUS_TYPE || mes.bus_type == ICACHE_BUS_TYPE 
                    || mes.bus_type == PTW_BUS_TYPE || mes.bus_type == DMA_BUS_TYPE){
                reset();  
                return false;
            }else{
                return false;
            }
        }
    // only storage the mes of first message link
    }else if(!link.empty() && index != first_col_index && (mes_type == ACK || mes_type == ACK1) ){
        return false;
    // if module send REQ in state write wait time
    }else if(!link.empty() && index == first_col_index && mes_type == REQ){
        reset();
        return false;
    }

    // storage first require index
    if(link.empty()){
        if(mes_type == REQ){
            first_col_index = index;
        }else{
            return false;
        }
    }

    //---------------Updata Link----------------//
    // storage previous value
    if(link.count(index) > 0)
        col = link[index];
    // insert mes and state to col
    switch (mes_type)
    {
        case REQ: col.Mes_Req = mes; if(pk.state.valid) col.State_b = state; break;
        case ACK: col.Mes_Ack = mes; if(pk.state.valid) col.State_e = state; break;
        case ACK1: col.Mes_Ack1 = mes; if(pk.state.valid) col.State_e = state; break;
        default: tlc_assert(false,"Illegal mes type!"); break;
    }
    // insert
    link[index] = col;


    //-----------------check finish--------------//
    return check_finish();
}

bool link::check_finish(){
    if(link.empty())
        return false;
    
    tlMes first_req = link[first_col_index].Mes_Req;
    // check GrantAck
    if(first_req.chnl == CHNLA && ( first_req.opcode == AcquireBlock || first_req.opcode == AcquirePerm) ){
        if(link[first_col_index].Mes_Ack1.valid == true){
            
            print();
            // reset();
            printf("LINK RESET\n");
            return true;
        }
    }
    // check others
    else{
        if(link[first_col_index].Mes_Ack.valid == true){
            
            print();
            // reset();
            printf("LINK RESET\n");
            return true;
        }
    }

    return false;
}

//-------------------------MesCom-----------------------//

void Mes_Com::arbiter(package pk){
    if(pk.mes.valid && pk.mes.address != 0x0){
        bool done;
        done = queue[pk.mes.address].updata_first(pk);
        if(done){
            link_col col = queue[pk.mes.address].get_first_col();
            report->record(col.Mes_Req, col.State_b, col.State_e);
            queue.erase(pk.mes.address);
        }      
    }
 }

//------------------------------------------------------//


}// namespace Cover