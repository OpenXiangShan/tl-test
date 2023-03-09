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
                                        {"verbose", 0, NULL, 'v'},
                                        {"fullwave", 0, NULL, 'f'},
                                        {"monitor", 0, NULL, 'm'},
                                        {"random", 0, NULL, 'r'},
                                        {0, 0, NULL, 0}};
  int o;
  int long_index = 0;
  while ((o = getopt_long(argc, const_cast<char *const *>(argv), "-s:b:e:c:mvfr",
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
    case 'v':
      Verbose = true;
      break;
    case 'm':
      en_monitor = true;
      break;
    case 'f':
      all_wave = true;
      break;
    case 'r':
      random_mode = false;
      break;
    default:
      tlc_assert(false, "Unknown args!");
    }
  }
  if(all_wave){
    this->wave_begin = 0;
    this->wave_end = this->exe_cycles;
  }
  if ((this->wave_begin >= this->wave_end)) {
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
  
  for (int i = 0; i < NR_CAGENTS; i++) {
    l1[i].reset(new FakeL1_t(globalBoard, i, &Cycles, i / 2, i % 2));
  }
  
  for(int i = NR_CAGENTS; i < NR_CAGENTS + NR_PTWAGT; i++) {
    ptw[i-NR_CAGENTS].reset(new FakePTW_t(globalBoard, i, &Cycles, i % 2, PTW_BUS_TYPE));
  }

  for(int i = NR_CAGENTS + NR_PTWAGT; i < NR_CAGENTS + NR_PTWAGT + NR_DMAAGT; i++) {
    dma[i-NR_CAGENTS-NR_PTWAGT].reset(new FakeDMA_t(globalBoard, i, &Cycles, 0xffffffffffffffffL, DMA_BUS_TYPE));
  }

  sqr.reset(new Sequencer_t(&Cycles));

  if (random_mode == false)
  {
    sqr->init_testcase(); // Input file test.txt
  }
  

  if(this->en_monitor){
    for (int i = 0; i < NR_TILE_MONITOR; i++) {
      monitors[i].reset(new tl_monitor::Monitor(&Cycles, i, TILE_BUS_TYPE));
    }

    for (int i = NR_TILE_MONITOR; i < NR_TILE_MONITOR + NR_L3_MONITOR; i++) {
      monitors[i].reset(new tl_monitor::Monitor(&Cycles, i - NR_TILE_MONITOR, L3_BUS_TYPE));
    }

    for (int i = NR_TILE_MONITOR + NR_L3_MONITOR; i < NR_TL_MONITOR; i++) {
      monitors[i].reset(new tl_monitor::Monitor(&Cycles, i - NR_TILE_MONITOR - NR_L3_MONITOR, DMA_BUS_TYPE));
    }

    for (int i = 0; i < NR_DIR_MONITOR; i++) {
      dir_monitors[i].reset(new DIR_monitor::DIR_Monitor(&Cycles, i, DIR_BUS_TYPE));
    }
  }
#if VM_TRACE == 1
  if (this->enable_wave) {
    Verilated::traceEverOn(true); // Verilator must compute traced signals
    // tfp = new VerilatedVcdC;
    tfp = new VerilatedFstC;
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

//reset_sys
void Emu::reset_sys(uint64_t n) {
    reset(n);
    this->globalBoard->clear();
    for(int i = 0; i < NR_CAGENTS; i++) {
      l1[i]->clear();
    }
    for(int i = 0; i < NR_PTWAGT; i++) {
      ptw[i]->clear();
    }
    for(int i = 0; i < NR_DMAAGT; i++) {
      dma[i]->clear();
    }
}

void Emu::execute(uint64_t nr_cycle) {
  while (Cycles < nr_cycle) {
    if(this->en_monitor){
      for(int i = 0; i < NR_TL_MONITOR; i++){
        monitors[i]->print_info();
      }
      for(int i = 0; i < NR_DIR_MONITOR; i++){
        dir_monitors[i]->print_info();
      }
    }

    if(sqr->do_reset(Cycles)){
      reset_sys(10);
    }
    
    for (int i = 0; i < NR_CAGENTS; i++) {
      l1[i]->handle_channel();
    }
    for (int i = 0; i < NR_PTWAGT; i++) {
      ptw[i]->handle_channel();
    }
    for (int i = 0; i < NR_DMAAGT; i++) {
      dma[i]->handle_channel();
    }

    if(random_mode) {
      for (int i = 0; i < NR_CAGENTS; i++) {
        // tl_base_agent::TLCTransaction tr = randomTest2(false, l1[i]->bus_type, ptw, dma);
        tl_base_agent::TLCTransaction tr = sqr->random_test_fullsys(sequencer::TLC, false, l1[i]->bus_type, ptw, dma);
        l1[i]->transaction_input(tr);
      }
      for (int i = 0; i < NR_PTWAGT; i++) {
        // tl_base_agent::TLCTransaction tr = randomTest3(ptw, dma, l1, ptw[i]->bus_type);
        tl_base_agent::TLCTransaction tr = sqr->random_test_fullsys(sequencer::TLUL, false, ptw[i]->bus_type, ptw, dma);
        ptw[i]->transaction_input(tr);
      }
      for (int i = 0; i < NR_DMAAGT; i++) {
        // tl_base_agent::TLCTransaction tr = randomTest3(ptw, dma, l1, dma[i]->bus_type);
        tl_base_agent::TLCTransaction tr = sqr->random_test_fullsys(sequencer::TLUL, false, dma[i]->bus_type, ptw, dma);
        dma[i]->transaction_input(tr);
      }
    }else{
      for (int i = 0; i < NR_CAGENTS; i++) {
        tl_base_agent::TLCTransaction tr = sqr->case_test(sequencer::TLC, i);
        l1[i]->transaction_input(tr);
      }
      for (int i = NR_CAGENTS; i < NR_CAGENTS + NR_PTWAGT; i++) {
        tl_base_agent::TLCTransaction tr = sqr->case_test(sequencer::TLUL, i);
        ptw[i-NR_CAGENTS]->transaction_input(tr);
      }
      for (int i = NR_CAGENTS + NR_PTWAGT; i < NR_CAGENTS + NR_PTWAGT + NR_DMAAGT; i++) {
        tl_base_agent::TLCTransaction tr = sqr->case_test(sequencer::TLUL, i);
        dma[i-NR_CAGENTS-NR_PTWAGT]->transaction_input(tr);
      }
    }

    

    for (int i = 0; i < NR_CAGENTS; i++) {
      l1[i]->update_signal();
    }
    for (int i = 0; i < NR_PTWAGT; i++) {
      ptw[i]->update_signal();
    }
    for (int i = 0; i < NR_DMAAGT; i++) {
      dma[i]->update_signal();
    }

    this->step();
    this->update_cycles(1);
  }
}
