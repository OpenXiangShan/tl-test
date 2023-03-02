//
// Created by zcy on 2023/2/27.
//

#ifndef FAKE_L1_H
#define FAKE_L1_H

#include "../../Utils/Common.h"
#include "../../Utils/ScoreBoard.h"
#include "../../TLAgent/BaseAgent.h"
#include "../../TLAgent/CAgent.h"
#include "../../TLAgent/ULAgent.h"
#include "../../Interface/Interface.h"
#include "../base_cache_model.h"
#include "../tl_c_agent.h"

// protocal related namespace
using namespace tl_agent;

namespace fake_l1 {

class FakeL1: public base_cache_model::BaseCacheModel<tl_interface::TLInfo> {

    public:
        FakeL1(GlobalBoard<paddr_t> *const gb, int id,uint64_t *cycles, uint64_t core_id, int bus_type);
        ~FakeL1();
        
        void transaction_input(tl_base_agent::TLCTransaction tr);
        void handle_channel(); // handle channel input transcations 
        void update_signal(); // drive channel to send transactions
        
        // release memory
        // this will clear all allocated shared_ptr
        void clear() {
            this->cache_info->clear();
            this->a_idpool.clear();
            this->c_idpool.clear();
            this->a_mshr_info->clear();
            this->b_mshr_info->clear();
            this->c_mshr_info->clear();
        }

        // std::string type_to_string() {
        //     using namespace std;
        //     string mhartid = "core " + to_string(this->core_id);
        //     string type;
        //     if(this->bus_type == DCACHE_BUS_TYPE) type = " d$";
        //     else type = " i$";
        //     return mhartid + type;
        // }

        // TODO： this function should be placed in private field
        bool local_probe(paddr_t address);
        
    private:    
        // local cache model info
        std::shared_ptr<ScoreBoard<paddr_t, C_SBEntry>> cache_info;
        // agent info
        std::shared_ptr<tl_c_agent::TLCAgent> agent;
        // tilelink info
        std::shared_ptr<tl_interface::TLInfo> &tl_info = bus_info; // rename bus info

        /* protocal info */ 
        // every transaction owns an unique id
        // notice that when we send a transaction there will be an id being allcated
        IDPool a_idpool; 
        IDPool c_idpool;
        // mshrs: store some mapped info of every transcation which is already allocated with an id
        std::shared_ptr<ScoreBoard<int, C_IDEntry>> a_mshr_info; // acquire
        std::shared_ptr<ScoreBoard<int, C_IDEntry>> b_mshr_info; // probe ack
        std::shared_ptr<ScoreBoard<int, C_IDEntry>> c_mshr_info; // release

        // cache model supported transactions
        bool do_acquireBlock(paddr_t address, int param, int alias);
        bool do_acquirePerm(paddr_t address, int param, int alias);
        bool do_releaseData(paddr_t address, int param, std::shared_ptr<uint8_t[]>data, int alias);
        bool do_releaseDataAuto(paddr_t address, int alias);
        

        void connect(); // connect port info to DPI-C interface
        void timeout_check();
};


} // namespace fake_l1


#endif // FAKE_L1_H