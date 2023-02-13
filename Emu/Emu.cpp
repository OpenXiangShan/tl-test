//
// Created by ljw on 10/22/21.
//

#include "Emu.h"

uint64_t Cycles;
bool Verbose = false;

double sc_time_stamp() { return 0; }

void Emu::parse_args(int argc, char **argv) {
  const struct option long_options[] = {{"seed", 1, NULL, 's'},
                                        {"wave-begin", 1, NULL, 'b'},
                                        {"wave-end", 1, NULL, 'e'},
                                        {"cycles", 1, NULL, 'c'},
                                        {"wave-full", 0, NULL, 'f'},
                                        {"verbose", 0, NULL, 'v'},
                                        {0, 0, NULL, 0}};
  int o;
  int long_index = 0;
  while ((o = getopt_long(argc, const_cast<char *const *>(argv), "-s:b:e:c:fv",
                          long_options, &long_index)) != -1) {
    switch (o) {
    case 's':
      this->seed = atoll(optarg);
      break;
    case 'b':
      this->wave_begin = atoll(optarg);
      break;
    case 'e':
      this->wave_end = atoll(optarg);
      break;
    case 'c':
      this->exe_cycles = atoll(optarg);
      break;
    case 'f':
      this->wave_full = true;
      break;
    case 'v':
      Verbose = true;
      break;
    default:
      tlc_assert(false, "Unknown args!");
    }
  }
  if (this->wave_begin >= this->wave_end) {
    this->enable_wave = false;
  }
}

Emu::Emu(int argc, char **argv) {
  this->parse_args(argc, argv);
  Verilated::commandArgs(argc, argv);
  Cycles = 0;
  dut_ptr = new Vtb_top();
  globalBoard = new GlobalBoard<paddr_t>(); // address indexed

  printf("[INFO] use seed: %ld\n", this->seed);
  srand(this->seed);

  // Init agents
  for (int i = 0; i < NR_CAGENTS; i++) {
    agents[i].reset(new CAgent_t(globalBoard, i, &Cycles, i / 2, i % 2));
    fuzzers[i].reset(new CFuzzer(std::dynamic_pointer_cast<tl_agent::CAgent>(agents[i])));
    fuzzers[i]->set_cycles(&Cycles);
  }

  // for (int i = NR_CAGENTS; i < NR_AGENTS; i++) {
  //   agents[i] = new ULAgent_t(globalBoard, i, &Cycles);
  //   auto port = naive_gen_port_dma();
  //   agents[i]->connect(port);
  //   fuzzers[i] = new ULFuzzer(static_cast<ULAgent_t *>(agents[i]));
  //   fuzzers[i]->set_cycles(&Cycles);
  // }

#if VM_TRACE == 1
  if (this->enable_wave) {
    Verilated::traceEverOn(true); // Verilator must compute traced signals
    tfp = new VerilatedVcdC;
    dut_ptr->trace(tfp, 99); // Trace 99 levels of hierarchy
    time_t now = time(NULL);
    tfp->open(cycle_wavefile(Cycles, now));
  }
#endif
}

Emu::~Emu() {
  delete dut_ptr;
#if VM_TRACE == 1
  if (this->enable_wave) {
    this->tfp->close();
  }
#endif
}

void Emu::execute(uint64_t nr_cycle) {
  while (Cycles < nr_cycle) {
    for (int i = 0; i < NR_AGENTS; i++) {
      agents[i]->handle_channel();
    }

    for (int i = 0; i < NR_AGENTS; i++) {
      fuzzers[i]->tick(agents);
    }

    for (int i = 0; i < NR_AGENTS; i++) {
      agents[i]->update_signal();
    }

    this->step();
    this->update_cycles(1);
  }
}

// tl_agent::Port<tl_agent::ReqField, tl_agent::RespField, tl_agent::EchoField,
//                BEATSIZE> *
// Emu::naive_gen_port_dma() {
//   auto port = new tl_agent::Port<tl_agent::ReqField, tl_agent::RespField,
//                                  tl_agent::EchoField, BEATSIZE>();

//   port->a.ready = &(dut_ptr->master_port_2_0_a_ready);
//   port->a.valid = &(dut_ptr->master_port_2_0_a_valid);
//   port->a.opcode = &(dut_ptr->master_port_2_0_a_bits_opcode);
//   port->a.param = &(dut_ptr->master_port_2_0_a_bits_param);
//   port->a.address = &(dut_ptr->master_port_2_0_a_bits_address);
//   port->a.size = &(dut_ptr->master_port_2_0_a_bits_size);
//   port->a.source = &(dut_ptr->master_port_2_0_a_bits_source);
//   port->a.mask = &(dut_ptr->master_port_2_0_a_bits_mask);
//   port->a.data = (uint8_t *)&(dut_ptr->master_port_2_0_a_bits_data);

//   port->d.ready = &(dut_ptr->master_port_2_0_d_ready);
//   port->d.valid = &(dut_ptr->master_port_2_0_d_valid);
//   port->d.opcode = &(dut_ptr->master_port_2_0_d_bits_opcode);
//   port->d.param = &(dut_ptr->master_port_2_0_d_bits_param);
//   port->d.size = &(dut_ptr->master_port_2_0_d_bits_size);
//   port->d.sink = &(dut_ptr->master_port_2_0_d_bits_sink);
//   port->d.source = &(dut_ptr->master_port_2_0_d_bits_source);
//   port->d.data = (uint8_t *)&(dut_ptr->master_port_2_0_d_bits_data);

//   return port;
// }
