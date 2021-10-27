//
// Created by ljw on 10/22/21.
//

#include "Emu.h"

Emu::Emu(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    cycles = 0;
    dut_ptr = new VTestTop();
    globalBoard = new ScoreBoard<uint64_t, std::array<uint8_t, 64>>(); // address -> data

    // Init agents
    for (int i = 0; i < NR_ULAGENTS; i++) {
        agents[i] = new ULAgent_t(globalBoard);
        auto port = naive_gen_port();
        agents[i]->connect(port);
    }
    for (int i = NR_ULAGENTS; i < NR_AGENTS; i++) {
        // TODO: init tl-c agents
    }

#if VM_TRACE == 1
    Verilated::traceEverOn(true);	// Verilator must compute traced signals
    tfp = new VerilatedVcdC;
    dut_ptr->trace(tfp, 99);	// Trace 99 levels of hierarchy
    time_t now = time(NULL);
    tfp->open(cycle_wavefile(cycles, now));
#endif

}

Emu::~Emu() {
    delete dut_ptr;
#if VM_TRACE == 1
    this->tfp->close();
#endif
}

void Emu::execute(uint64_t nr_cycle) {
    while (cycles < nr_cycle) {
        // TODO: to be removed
        if (cycles == 100) {
            dynamic_cast<ULAgent_t*>(agents[0])->do_get(0x1000);
        }
        for (int i = 0; i < NR_AGENTS; i++) {
            agents[i]->update();
        }

        this->neg_edge();
#if VM_TRACE == 1
        this->tfp->dump((vluint64_t)cycles);
#endif
        this->pos_edge();
        this->update_cycles(1);
    }
}

// the following code is to be replaced soon, only for test
Port<ReqField, RespField, EchoField, DATASIZE>* Emu::naive_gen_port() {
    auto port = new Port<ReqField, RespField, EchoField, DATASIZE>();
    port->a.ready = &(dut_ptr->master_port_0_0_a_ready);
    port->a.valid = &(dut_ptr->master_port_0_0_a_valid);
    port->a.opcode = &(dut_ptr->master_port_0_0_a_bits_opcode);
    port->a.address = &(dut_ptr->master_port_0_0_a_bits_address);
    port->a.size = &(dut_ptr->master_port_0_0_a_bits_size);
    port->a.source = &(dut_ptr->master_port_0_0_a_bits_source);
    port->a.mask = &(dut_ptr->master_port_0_0_a_bits_mask);
    port->a.source = &(dut_ptr->master_port_0_0_a_bits_source);
    port->d.ready = &(dut_ptr->master_port_0_0_d_ready);
    port->d.valid = &(dut_ptr->master_port_0_0_d_valid);
    port->d.opcode = &(dut_ptr->master_port_0_0_d_bits_opcode);
    port->d.param = &(dut_ptr->master_port_0_0_d_bits_param);
    port->d.size = &(dut_ptr->master_port_0_0_d_bits_size);
    port->d.source = &(dut_ptr->master_port_0_0_d_bits_source);
    return port;
}