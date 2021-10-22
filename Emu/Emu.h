//
// Created by ljw on 10/22/21.
//

#ifndef TLC_TEST_EMU_H
#define TLC_TEST_EMU_H

#include "verilated.h"
#include "VTestTop.h"

class Emu {
private:
    VTestTop *dut_ptr;
    uint64_t cycles;
public:
    Emu(int argc, char **argv);
    ~Emu();
    inline void reset(uint64_t n);
    inline void neg_edge();
    inline void pos_edge();
    inline void update_cycles(uint64_t inc);
};

inline void Emu::reset(uint64_t n) {
    for(uint64_t i = 0; i < n; i++){
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

#endif //TLC_TEST_EMU_H
