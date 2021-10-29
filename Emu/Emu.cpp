//
// Created by ljw on 10/22/21.
//

#include "Emu.h"

Emu::Emu(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    cycles = 0;
    dut_ptr = new VTestTop();
    globalBoard = new GlobalBoard<uint64_t>(); // address -> data

    // Init agents
    for (int i = 0; i < NR_ULAGENTS; i++) {
        agents[i] = new ULAgent_t(globalBoard, &cycles);
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
    srand((unsigned)time(0));
    while (cycles < nr_cycle) {
        for (int i = 0; i < NR_AGENTS; i++) {
            agents[i]->handle_channel();
        }
        if (cycles == 100) {
            dynamic_cast<ULAgent_t*>(agents[0])->do_get(0x1000);
        }
        if (cycles == 200) {
            uint8_t* putdata = new uint8_t[DATASIZE];
            for (int i = 0; i < DATASIZE; i++) {
                putdata[i] = (uint8_t)rand();
            }
            dynamic_cast<ULAgent_t*>(agents[0])->do_putfulldata(0x2000, putdata);
        }
        if (cycles == 202) {
            dynamic_cast<ULAgent_t*>(agents[0])->do_get(0x2000);
        }

        for (int i = 0; i < NR_AGENTS; i++) {
            agents[i]->update_signal();
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
tl_agent::Port<tl_agent::ReqField, tl_agent::RespField, tl_agent::EchoField, BEATSIZE>* Emu::naive_gen_port() {
    auto port = new tl_agent::Port<tl_agent::ReqField, tl_agent::RespField, tl_agent::EchoField, BEATSIZE>();
    port->a.ready = &(dut_ptr->master_port_0_0_a_ready);
    port->a.valid = &(dut_ptr->master_port_0_0_a_valid);
    port->a.opcode = &(dut_ptr->master_port_0_0_a_bits_opcode);
    port->a.address = &(dut_ptr->master_port_0_0_a_bits_address);
    port->a.size = &(dut_ptr->master_port_0_0_a_bits_size);
    port->a.source = &(dut_ptr->master_port_0_0_a_bits_source);
    port->a.mask = &(dut_ptr->master_port_0_0_a_bits_mask);
    port->a.source = &(dut_ptr->master_port_0_0_a_bits_source);
    port->a.data = (uint8_t*)&(dut_ptr->master_port_0_0_a_bits_data);
    port->d.ready = &(dut_ptr->master_port_0_0_d_ready);
    port->d.valid = &(dut_ptr->master_port_0_0_d_valid);
    port->d.opcode = &(dut_ptr->master_port_0_0_d_bits_opcode);
    port->d.param = &(dut_ptr->master_port_0_0_d_bits_param);
    port->d.size = &(dut_ptr->master_port_0_0_d_bits_size);
    port->d.source = &(dut_ptr->master_port_0_0_d_bits_source);
    port->d.data = (uint8_t*)&(dut_ptr->master_port_0_0_d_bits_data);
    return port;
}