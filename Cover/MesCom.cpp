#include "MesCom.h"

namespace Cover{

using namespace tl_agent;

//---------------------link------------------------//

void link::print(){
    link_index index;
    for (auto [key, val] : link) {
        printf("REQ: opcode [%d] param [%d] address [%lx]\n", val.Mes_Req.opcode, val.Mes_Req.param, val.Mes_Req.address);
        printf("ACK: opcode [%d] param [%d] address [%lx]\n", val.Mes_Ack.opcode, val.Mes_Ack.param, val.Mes_Ack.address);
        printf("ACK1: opcode [%d] param [%d] address [%lx]\n", val.Mes_Ack1.opcode, val.Mes_Ack1.param, val.Mes_Ack1.address);
    }
    
}


// tlMes Matcher(tlMes mes){
//     // level
//     uint8_t level;
//     switch (mes.bus_type){
//         case DCACHE_BUS_TYPE    : level = LEVEL_L1L2;  break;
//         case ICACHE_BUS_TYPE    : level = LEVEL_L1L2; break;
//         case PTW_BUS_TYPE       : level = LEVEL_L1L2; break;
//         case TILE_BUS_TYPE      : level = LEVEL_L2L3; break;
//         case DMA_BUS_TYPE       : level = LEVEL_L2L3; break;
//         case L3_BUS_TYPE        : level = LEVEL_L3MEM; break;
//         default: tlc_assert(false,"ERROR LEVEL\n");
//     }
//     // next level
//     uint8_t next_level_up = level;
//     uint8_t next_level_down = level+1;

    
//     //insert
//     const uint8_t dont_care = 0;
//     std::set<tlMes> legal_next_Mes_d;
//     std::set<tlMes> legal_next_Mes_u;
//     tlMes mes_be_insert;
//     //DCache : Acquire* ReleaseData    
//     if(mes.bus_type == DCACHE_BUS_TYPE){
//         //down
//         mes_be_insert.core_id = mes.core_id;
//         mes_be_insert.opcode = mes.opcode;// Acquire* or ReleaseData
//         mes_be_insert.bus_type = TILE_BUS_TYPE;
//         legal_next_Mes_d.insert(mes_be_insert);

//         //up
//         if(mes.opcode == AcquireBlock || mes.opcode == AcquirePerm){
//             mes_be_insert.core_id = mes.core_id;
//             mes_be_insert.opcode = Probe;
//             mes_be_insert.bus_type = ICACHE_BUS_TYPE;
//             legal_next_Mes_u.insert(mes_be_insert);
//         }
//     }
//     //ICache : AcquireBlock ReleaseData 
//     else if(mes.bus_type == ICACHE_BUS_TYPE){
//         //down
//         mes_be_insert.core_id = mes.core_id;
//         mes_be_insert.opcode = mes.opcode;
//         mes_be_insert.bus_type = TILE_BUS_TYPE;
//         legal_next_Mes_d.insert(mes_be_insert);

//         //up
//         if(mes.opcode == AcquireBlock){
//             mes_be_insert.core_id = mes.core_id;
//             mes_be_insert.opcode = Probe;
//             mes_be_insert.bus_type = DCACHE_BUS_TYPE;
//             legal_next_Mes_u.insert(mes_be_insert);
//         }
//     }
//     //PTW : Get
//     else if(mes.bus_type == PTW_BUS_TYPE){
//         //down
//         mes_be_insert.core_id = mes.core_id;
//         mes_be_insert.opcode = mes.opcode;//Get
//         mes_be_insert.bus_type = TILE_BUS_TYPE;
//         legal_next_Mes_d.insert(mes_be_insert);
//         mes_be_insert.opcode = AcquireBlock;
//         legal_next_Mes_d.insert(mes_be_insert);
//         //up
//         if(mes.opcode == Get){
//             mes_be_insert.core_id = mes.core_id;
//             mes_be_insert.opcode = Probe;
//             mes_be_insert.bus_type = DCACHE_BUS_TYPE;
//             legal_next_Mes_u.insert(mes_be_insert);
//             mes_be_insert.bus_type = DCACHE_BUS_TYPE;
//             legal_next_Mes_u.insert(mes_be_insert);
//         }
//     }
//     //L2 : Acquire* Release Get
//     else if(mes.bus_type == TILE_BUS_TYPE){
//         //down
//         if(mes.opcode == ReleaseData){
//             mes_be_insert.core_id = dont_care;
//             mes_be_insert.opcode = mes.opcode;// ReleaseData
//             mes_be_insert.bus_type = L3_BUS_TYPE;
//             legal_next_Mes_d.insert(mes_be_insert);
//         }else if(mes.opcode == Get){
//             mes_be_insert.core_id = dont_care;
//             mes_be_insert.opcode = mes.opcode;// Get
//             mes_be_insert.bus_type = L3_BUS_TYPE;
//             legal_next_Mes_d.insert(mes_be_insert);
//             mes_be_insert.opcode = mes.opcode;// AcquireBlock
//             legal_next_Mes_d.insert(mes_be_insert);
//         }else if(mes.opcode == AcquireBlock || mes.opcode == AcquirePerm){
//             mes_be_insert.core_id = dont_care;
//             mes_be_insert.opcode = mes.opcode;// Acquire*
//             mes_be_insert.bus_type = L3_BUS_TYPE;
//             legal_next_Mes_d.insert(mes_be_insert);
//         }

//         //up
//         if(mes.opcode != ReleaseData){
//             if(mes.core_id == ID_CORE0){
//                 mes_be_insert.core_id = ID_CORE1;
//             }else if(mes.core_id == ID_CORE1){
//                 mes_be_insert.core_id = ID_CORE0;
//             }
//             mes_be_insert.opcode = Probe;
//             mes_be_insert.bus_type = mes.bus_type;
//             legal_next_Mes_u.insert(mes_be_insert);
//         }
//     }
//     //DMA : Get Put*
//     else if(mes.bus_type == DMA_BUS_TYPE){
//         //down
//         mes_be_insert.core_id = mes.core_id;
//         mes_be_insert.opcode = mes.opcode;// Get or Put*
//         mes_be_insert.bus_type = TILE_BUS_TYPE;
//         legal_next_Mes_d.insert(mes_be_insert);
//         mes_be_insert.opcode = AcquireBlock;// AcquireBlock* 
//         legal_next_Mes_d.insert(mes_be_insert);
//         //up Get Put*
//         mes_be_insert.core_id = ID_CORE0;
//         mes_be_insert.opcode = Probe;
//         mes_be_insert.bus_type = TILE_BUS_TYPE;
//         legal_next_Mes_u.insert(mes_be_insert);
//         mes_be_insert.core_id = ID_CORE1;
//         legal_next_Mes_u.insert(mes_be_insert);
//     }
//     else if(mes.bus_type == L3_BUS_TYPE){
//         // None
//     }else

// }


// bool link::check_req(tlMes mes){
//     link_index index;
//     index.bus_type = mes.bus_type;
//     index.core_id = mes.core_id;

//     // req is the first req of this link no need to check
//     if(link.count(index) == 0){
//         return true;
//     }

//     switch (mes.bus_type){
//         // case DCACHE_BUS_TYPE    :  break;
//         // case ICACHE_BUS_TYPE    : level = LEVEL_L1L2; break;
//         // case PTW_BUS_TYPE       : level = LEVEL_L1L2; break;
//         // case TILE_BUS_TYPE      : level = LEVEL_L2L3; break;
//         // case DMA_BUS_TYPE       : level = LEVEL_L2L3; break;
//         // case L3_BUS_TYPE        : level = LEVEL_L3MEM; break;
//         default: tlc_assert(false,"ERROR BUS_TYPE\n");
//     }

    

// }

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
        case REQ: col.Mes_Req = mes; if(pk.state.valid) col.State_b = state; printf("type switch\n"); break;
        case ACK: col.Mes_Ack = mes; if(pk.state.valid) col.State_e = state; printf("type switch\n"); break;
        case ACK1: col.Mes_Ack1 = mes; if(pk.state.valid) col.State_e = state; printf("type switch\n"); break;
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

bool link::check_finish(){
    if(link.empty())
        return false;
    printf("CHECK FINISH!\n");
    tlMes first_req = link[first_col_index].Mes_Req;
    // check GrantAck
    if(first_req.opcode == AcquireBlock || first_req.opcode == AcquirePerm){
        if(link[first_col_index].Mes_Ack1.valid == true){
            
            print();
            reset();
            printf("LINK RESET\n");
            return true;
        }
    }
    // check others
    else{
        if(link[first_col_index].Mes_Ack1.valid == true){
            
            print();
            reset();
            printf("LINK RESET\n");
            return true;
        }
    }

    return false;
}

//-------------------------MesCom-----------------------//

void Mes_Com::arbiter(package pk){
    if(pk.mes.valid){
        queue[pk.mes.address].update(pk);
        printf("Arbiter Ture\n");
    }
}

//------------------------------------------------------//


}// namespace Cover