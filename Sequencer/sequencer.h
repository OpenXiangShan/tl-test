//
// Created by zcy on 2023/2/27.
//
#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "../Utils/Common.h"
#include "../Utils/ScoreBoard.h"
#include "../TLAgent/BaseAgent.h"
#include "../Interface/Interface.h"
#include "../CacheModel/base_cache_model.h"
#include "../CacheModel/tl_base_agent.h"
#include "../CacheModel/FakeL1/fake_l1.h"
#include "../CacheModel/FakePTW/fake_ptw.h"
#include "../Fuzzer/Case.h"

namespace sequencer {

enum {
    TLC = 0,
    TLUL,
};

class Sequencer {
    public:
        Sequencer(uint64_t *cycles) { this->cycles = cycles; };
        ~Sequencer() {};
        /* Random Test Part */
        // fullsystem simulation, for now we support 【L1 / PTW(Fake) --> L2 / DMA(Fake) -->L3 --> Mem】
        tl_base_agent::TLCTransaction random_test_fullsys(uint8_t tr_type, bool do_alias, uint8_t bus_type,
                                                                std::shared_ptr<fake_ptw::FakePTW> *ptw, 
                                                                std::shared_ptr<fake_ptw::FakePTW> *dma);
        
        /* Case Test Part... */ 
        void init_testcase(void) {
            testcase::read_file rt;
            rt.read_test(case_mes);
        }

        bool do_reset(uint64_t Cycles){
            if(case_mes[0].haskey(Cycles)){
                if(case_mes[0].query(Cycles).opcode == testcase::reset_opcode){
                    std::cout << "System: "<< "[" << Cycles << "] " << "reset" << std::endl;
                    return true;
                }
            }
            return false;
        }

        tl_base_agent::TLCTransaction case_test(uint8_t tr_type, int id);

        
    private:
        uint64_t *cycles;
        testcase::Message case_mes[testcase::N_TESTCASE_AGENT];
};

}


#endif // SEQUENCER_H