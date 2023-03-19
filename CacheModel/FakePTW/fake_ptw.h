//
// Created by zcy on 2023/3/2.
//
#ifndef FAKE_PTW_H
#define FAKE_PTW_H

#include "../../Utils/Common.h"
#include "../../Utils/ScoreBoard.h"
#include "../../TLAgent/BaseAgent.h"
#include "../../TLAgent/CAgent.h"
#include "../../TLAgent/ULAgent.h"
#include "../../Interface/Interface.h"
#include "../base_cache_model.h"
#include "../tl_ul_agent.h"


using namespace tl_agent;

namespace fake_ptw {

class FakePTW: public base_cache_model::BaseCacheModel<tl_interface::TLInfo> {
    public:
        FakePTW(GlobalBoard<paddr_t> *const gb, int id,uint64_t *cycles, uint64_t core_id, int bus_type);
        ~FakePTW() {};

        void transaction_input(tl_base_agent::TLCTransaction tr);
        void handle_channel(); // handle channel input transcations 
        void update_signal(); // drive channel to send transactions

        void clear(){
            this->localBoard->clear();
        }

        bool local_probe(paddr_t address);

        std::shared_ptr<tl_interface::TLInfo> get_info();

    private:
        IDPool a_idpool;
        std::shared_ptr<tl_interface::TLInfo> &tl_info = bus_info;
        /* We only need a localBoard recording SourceID -> UL_SBEntry
        * because UL agent needn't store data.
        */
        ScoreBoard<int, UL_SBEntry> *localBoard; // SourceID -> UL_SBEntry

        std::shared_ptr<tl_ul_agent::TLULAgent> agent;

        void connect(); // connect port info to DPI-C interface
        void timeout_check();

        bool do_getAuto(paddr_t address);
        bool do_get(paddr_t address, uint8_t size, uint32_t mask);
        bool do_putfulldata(paddr_t address, std::shared_ptr<uint8_t[]> data);
        bool do_putpartialdata(paddr_t address, uint8_t offset, uint8_t size, uint32_t mask,
                                std::shared_ptr<uint8_t[]> data);
};

    


}

#endif // FAKE_PTW_H
