//
// Created by ljw on 10/22/21.
//

#ifndef TLC_TEST_EMU_H
#define TLC_TEST_EMU_H

#include "verilated.h"
#include "VTestTop.h"
#include "../Utils/ScoreBoard.h"
#include "../TLAgent/ULAgent.cpp"

class Emu {
private:
    // TODO: move out following parameters into a monolithic config
    enum {
        DATASIZE = 64, // Cache line is 64B
        NR_ULAGENTS = 1,
        NR_CAGENTS = 0,
    };
    typedef tl_agent::BaseAgent<tl_agent::ReqField, tl_agent::RespField, tl_agent::EchoField, DATASIZE> BaseAgent_t;
    typedef tl_agent::ULAgent<tl_agent::ReqField, tl_agent::RespField, tl_agent::EchoField, DATASIZE> ULAgent_t;

    const static int NR_AGENTS = NR_CAGENTS + NR_ULAGENTS;
    VTestTop *dut_ptr;
    ScoreBoard<std::array<uint8_t, DATASIZE>> *globalBoard;
    BaseAgent_t ** const agents = new BaseAgent_t*[NR_AGENTS];
    uint64_t cycles;

public:
    Emu(int argc, char **argv);
    ~Emu();
    inline void reset(uint64_t n);
    inline void neg_edge();
    inline void pos_edge();
    inline void update_cycles(uint64_t inc);
    void execute(uint64_t nr_cycle);
};

inline void Emu::reset(uint64_t n) {
    for (uint64_t i = 0; i < n; i++) {
        dut_ptr->reset = 1;
        dut_ptr->clock = 0;
        dut_ptr->eval();
        dut_ptr->clock = 1;
        dut_ptr->eval();
    }
    dut_ptr->reset = 0;
}

inline void Emu::neg_edge() {
    dut_ptr->clock = 0;
    dut_ptr->eval();
}

inline void Emu::pos_edge() {
    dut_ptr->clock = 1;
    dut_ptr->eval();
}

inline void Emu::update_cycles(uint64_t inc) {
    cycles += inc;
}

double sc_time_stamp() { return 0; }

#endif //TLC_TEST_EMU_H
