#include "MesCom.h"

namespace Cover{

using namespace tl_agent;

//---------------------link------------------------//

void link::print(){

}

void link::update(package pk){
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
    link.insert(std::make_pair(index, col));

    //check finish
    check_finish();
}

bool link::check_finish(){
    if(link.empty())
        return false;

    tlMes first_req = link[first_col_index].Mes_Req;
    // check GrantAck
    if(first_req.opcode == AcquireBlock || first_req.opcode == AcquirePerm){
        if(link[first_col_index].Mes_Ack1.valid == true){
            return true;
            print();
            reset();
        }
    }
    // check others
    else{
        if(link[first_col_index].Mes_Ack1.valid == true){
            return true;
            print();
            reset();
        }
    }

    return false;
}

//-------------------------MesCom-----------------------//

void Mes_Com::arbiter(package pk){
    if(pk.mes.valid){
        queue[pk.mes.address].update(pk);
    }
}

//------------------------------------------------------//


}// namespace Cover