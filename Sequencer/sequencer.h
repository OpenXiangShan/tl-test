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

namespace sequencer {

enum {
    TLC = 0,
    TLUL,
};

class Sequencer {
    public:
        Sequencer() {};
        ~Sequencer() {};
        tl_base_agent::TLCTransaction random_test(uint8_t tr_type, bool do_alias, uint8_t bus_type);
};

}


#endif // SEQUENCER_H