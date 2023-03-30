#pragma once

#ifndef TLC_TEST_UTILS_H
#define TLC_TEST_UTILST_H


#include "../Utils/Common.h"
#include "../TLAgent/Port.h"
#include "../Monitor/DIR_Monitor.h"

namespace Cover {

enum{
    ID_CORE0 = 0,
    ID_CORE1,
    ID_L3,
    ID_DMA,
    ID_MAX = 4
};

class tlMes{
public:
    uint8_t opcode;
    uint8_t param;
    paddr_t address;
    uint8_t chnl;
    uint64_t core_id;//only work for L1 L2
    uint8_t bus_type;
    bool valid;
};

class cacheState{
public:
    uint8_t L1[2][2];//[core_id][bus_type]
    uint8_t L2[2];//[core_id]
    uint8_t L3;
    bool valid;
};

class package{
public:
    tlMes mes;
    cacheState state;
};


} // namespace Cover

#endif // TLC_TEST_UTILS_H