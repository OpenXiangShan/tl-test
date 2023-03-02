//
// Created by zcy on 2023/2/27.
//
#ifndef BASE_AGENT_H
#define BASE_AGENT_H

#include "../Utils/Common.h"
#include "../Utils/ScoreBoard.h"
#include "../TLAgent/BaseAgent.h"
#include "../TLAgent/CAgent.h"
#include "../TLAgent/ULAgent.h"
#include "../Interface/Interface.h"
#include "base_cache_model.h"

using namespace tl_agent;
namespace tl_base_agent {
    // opcode
    enum {
        ACQUIRE_PERM = 0,
        ACQUIRE_BLOCK,
        RELEASE_DATA,
        RELEASE_DATA_AUTO, // feel free to release data according to its priv
        GET_AUTO,
        GET,
        PUT_FULL_DATA,
        PUT_PARTIAL_DATA,
        INVALID_REQ
    };
    
    typedef uint8_t opcode_t;
    typedef int param_t;

    class TLCTransaction {
        public:
            opcode_t opcode;
            param_t param;
            paddr_t addr;
            std::shared_ptr<uint8_t[]>data;
            uint8_t alias;
            uint32_t mask;
            uint8_t offset;
            uint8_t size;
    };

    class TLBaseAgent {
        public:
            std::shared_ptr<Port<ReqField, RespField, EchoField, BEATSIZE>> port;

        public:
            int id;
            int core_id;
            int bus_type;
            uint64_t *cycles;
            
            TLBaseAgent(uint64_t *cycles, int id, int core_id, int bus_type): 
                cycles(cycles), id(id), core_id(core_id), bus_type(bus_type) {};
            
            std::string type_to_string() {
                using namespace std;
                string mhartid = "core " + to_string(this->core_id);
                string type;
                switch (this->bus_type) {
                    case DCACHE_BUS_TYPE: { type = " d$";  break; }
                    case ICACHE_BUS_TYPE: { type = " i$";  break; }
                    case TILE_BUS_TYPE:   { type = " L2$"; break; }
                    case L3_BUS_TYPE:     { type = " L3$"; break; }
                    default: break;
                }
                return mhartid + type;
            }
            // transaction input process functions
            virtual void fire_a() = 0;
            virtual void fire_b() = 0;
            virtual void fire_c() = 0;
            virtual void fire_d() = 0;
            virtual void fire_e() = 0;

            // these flags indicate whether channel has receive a fired transaction 
            bool a_fire = false;
            bool b_fire = false;
            bool c_fire = false;
            bool d_fire = false;
            bool e_fire = false;

            bool update_a() { if(a_fire) {a_fire = false; return true;} else return false;}
            bool update_b() { if(b_fire) {b_fire = false; return true;} else return false;}
            bool update_c() { if(c_fire) {c_fire = false; return true;} else return false;}
            bool update_d() { if(d_fire) {d_fire = false; return true;} else return false;}
            bool update_e() { if(e_fire) {e_fire = false; return true;} else return false;}
        

    
    };


}



#endif // BASE_AGENT_H