//
// Created by ljw on 10/22/21.
//

#include "Emu.h"

Emu::Emu(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    cycles = 0;
    dut_ptr = new VTestTop();
    globalBoard = new ScoreBoard<std::array<uint8_t, 64>>();

    // Init agents
    for (int i = 0; i < NR_ULAGENTS; i++) {
        agents[i] = new ULAgent_t(globalBoard);
    }
    for (int i = NR_ULAGENTS; i < NR_AGENTS; i++) {
        // TODO: init tl-c agents
    }
}

Emu::~Emu() {
    delete dut_ptr;
}

void Emu::execute(uint64_t nr_cycle) {
    while (cycles < nr_cycle) {
        for (int i = 0; i < NR_AGENTS; i++) {
            agents[i]->fire_a();
            agents[i]->fire_b();
            agents[i]->fire_c();
            agents[i]->fire_e();
        }
        this->neg_edge();
        this->pos_edge();
        this->update_cycles(1);
    }
}