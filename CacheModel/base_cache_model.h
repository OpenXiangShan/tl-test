//
// Created by zcy on 2023/2/27.
//

#ifndef CACHE_MODEL_H
#define CACHE_MODEL_H

#include "../Utils/Common.h"
#include "../Utils/ScoreBoard.h"
#include "../TLAgent/BaseAgent.h"
#include "../TLAgent/CAgent.h"
#include "../TLAgent/ULAgent.h"
#include "../Interface/Interface.h"


namespace base_cache_model {

    template <class BusInfo> 
    class BaseCacheModel {
        public:
            int bus_type;
            BaseCacheModel() {};
            ~BaseCacheModel() = default;
            virtual void transaction_input() {};
            virtual void handle_channel() {}; // handle channel input transcations 
            virtual void update_signal() {}; // drive channel to send transactions
            virtual void connect() {}; // connect port info to DPI-C interface 
            virtual void timeout_check() {};

            std::string type_to_string() {
                using namespace std;
                string mhartid = "core " + to_string(this->core_id);
                string type;
                switch (this->bus_type) {
                    case DCACHE_BUS_TYPE: { type = " d$";  break; }
                    case ICACHE_BUS_TYPE: { type = " i$";  break; }
                    case PTW_BUS_TYPE:    { type = " PTW"; break; }
                    case DMA_BUS_TYPE:    { type = "DMA";  mhartid = ""; break; }
                    case TILE_BUS_TYPE:   { type = " L2$"; break; }
                    case L3_BUS_TYPE:     { type = " L3$"; break; }
                    default: break;
                }
                return mhartid + type;
            }
        
        protected:
            int id;
            uint64_t core_id;
            GlobalBoard<paddr_t> *globalBoard; // global scoreboard
            uint64_t *cycles; // global cycles
            std::shared_ptr<BusInfo> bus_info;
        
    };

} // namespace cache_model


#endif // CACHE_MODEL_H