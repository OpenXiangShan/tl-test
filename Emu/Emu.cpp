//
// Created by ljw on 10/22/21.
//

#include "Emu.h"

Emu::Emu(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    cycles = 0;
    dut_ptr = new VTestTop();
}

Emu::~Emu() {
    delete dut_ptr;
}

