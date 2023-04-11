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

  HLOG(P_SW_T,"[INFO] use seed: %ld\n", this->seed);
  srand(this->seed);
  
  for (int i = 0; i < NR_CAGENTS; i++) {
    l1[i].reset(new FakeL1_t(globalBoard, i, &Cycles, i / 2, i % 2));// core_id 0 0 1 1 bus_type 0 1 0 1 -> D0 I0 D1 I1
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
    sqr->init_testcase_with_states();
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
      dir_monitors[i].reset(new DIR_monitor::DIR_Monitor(selfDir, selfTag, clientDir, clientTag, &Cycles, i, DIR_BUS_TYPE));
    }

    //cover
    report = new Cover::Report(seed, exe_cycles);
    mes_com = new Cover::Mes_Com(report);
    mes_collect.reset(new Cover::Mes_Collect(selfDir, selfTag, clientDir, clientTag, mes_com));
    
    // DIR_Write
    dir_write_0.reset(new dir_write::DIR_Write(DIR_WRITE_BUS_TYPE));
  }

  

#if VM_TRACE == 1
  if (this->enable_wave) {
    Verilated::traceEverOn(true); // Verilator must compute traced signals
    tfp = new VerilatedVcdC;
    // tfp = new VerilatedFstC;
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
    // reset dut
    reset(n);
    // reset board
    this->globalBoard->clear();
    // reset cache model
    for(int i = 0; i < NR_CAGENTS; i++) {
      l1[i]->clear();
    }
    for(int i = 0; i < NR_PTWAGT; i++) {
      ptw[i]->clear();
    }
    for(int i = 0; i < NR_DMAAGT; i++) {
      dma[i]->clear();
    }
    // reset dir
    for (int i = 0; i < 3; i++)
    {
      selfDir[i].clear();
      selfTag[i].clear();
      clientDir[i].clear();
      clientTag[i].clear();
    }
    HLOG(P_SW_T,"[%ld] sys reset done!\n", Cycles);
}

void Emu::execute(uint64_t nr_cycle) {
  while (Cycles < nr_cycle) {
    if(this->en_monitor){
      //----------DIR_Write----------//
      if(sqr->case_with_states.tc.count(Cycles+50) > 0){
        if(sqr->case_with_states.tc[Cycles+50].opcode != testcase_with_states::reset_opcode){
          dir_write_0->write(sqr->case_with_states.tc[Cycles+50].state, 0x80000000);
          // update glovalboard
          bool hasData = false;
          bool init_no_zero = false;
          for (uint8_t i = 0; i < 3; i++)
          {
            if(sqr->case_with_states.tc[Cycles+50].state.self[i] != INVALID
                || sqr->case_with_states.tc[Cycles+50].state.client[i][0] != INVALID
                || sqr->case_with_states.tc[Cycles+50].state.client[i][1] != INVALID)
              hasData = true;
          }
          // some l1 = Tip -> init data no zero
          for (uint8_t i = 0; i < 2; i++)
          {
            if(sqr->case_with_states.tc[Cycles+50].state.client[i][0] == TIP
                || sqr->case_with_states.tc[Cycles+50].state.client[i][1] == TIP)
              init_no_zero = true;
          }  
          // some cache != INVALID && acquire _toB -> init data no zero
          // for (uint8_t i = 0; i < 3; i++)
          // {
          //   if((sqr->case_with_states.tc[Cycles+50].state.self[i] != INVALID
          //       || sqr->case_with_states.tc[Cycles+50].state.client[i][0] != INVALID
          //       || sqr->case_with_states.tc[Cycles+50].state.client[i][1] != INVALID)
          //       && sqr->case_with_states.tc[Cycles+50].opcode == AcquireBlock 
          //       && sqr->case_with_states.tc[Cycles+50].param == NtoB
          //       && sqr->case_with_states.tc[Cycles+50].chnl == CHNLA)
          //     init_no_zero = false;
          // } 
          if(hasData){
            std::shared_ptr<Global_SBEntry> global_SBEntry(new Global_SBEntry());
            if(init_no_zero){
              global_SBEntry->data.reset(new uint8_t[DATASIZE]);
            }else{
              global_SBEntry->data.reset(new uint8_t[DATASIZE]);
              for (int i = 0; i < DATASIZE; i++) {
                global_SBEntry->data[i] = 0;
              }
            }
            global_SBEntry->status = Global_SBEntry::SB_VALID;
            global_SBEntry->mask = FULLMASK;
            globalBoard->update(0x80000000, global_SBEntry);
          }
          // update cache info
          for (uint8_t i = 0; i < 2; i++)// L2
          {
            uint64_t time_stamp = Cycles;
            int status[4] = {S_VALID};
            // DCACHE
            if(sqr->case_with_states.tc[Cycles+50].state.client[i][0] != INVALID){
              int privilege[4] = {sqr->case_with_states.tc[Cycles+50].state.client[i][0]};
              std::shared_ptr<C_SBEntry> entry(new C_SBEntry(status, privilege, time_stamp));
              l1[i*2+0]->update_cache_info(0x80000000, entry);
            }
            // ICACHE
            if(sqr->case_with_states.tc[Cycles+50].state.client[i][1] != INVALID){
              int privilege[4] = {sqr->case_with_states.tc[Cycles+50].state.client[i][1]};
              std::shared_ptr<C_SBEntry> entry(new C_SBEntry(status, privilege, time_stamp));
              l1[i*2+1]->update_cache_info(0x80000000, entry);
            }
          }
        }
      }else{
        dir_write_0->close();
      }        


      //-----------Monitor-----------//
      for(int i = 0; i < NR_TL_MONITOR; i++){
        monitors[i]->print_info();
      }
      for(int i = 0; i < NR_DIR_MONITOR; i++){
        dir_monitors[i]->print_info();
      }

      // //-----------Cover-----------//
      // for (int i = 0; i < NR_CAGENTS; i++) {//D$ & I$
      //   mes_collect->fire_Mes_Collect(l1[i]->get_info(),l1[i]->core_id,l1[i]->bus_type);
      // }
      // // for (int i = 0; i < NR_PTWAGT; i++) {//PTW
      // //   mes_collect->fire_Mes_Collect(ptw[i]->get_info(),ptw[i]->core_id,ptw[i]->bus_type);
      // // }
      // // for (int i = 0; i < NR_DMAAGT; i++) {//DMA
      // //   mes_collect->fire_Mes_Collect(dma[i]->get_info(),dma[i]->core_id,dma[i]->bus_type);
      // // }
      // for (int i = 0; i < NR_TL_MONITOR; i++) {//L2-L3 & L3-MEM
      //   mes_collect->fire_Mes_Collect(monitors[i]->get_info(),monitors[i]->id,monitors[i]->bus_type);
      // }

      // for (int i = 0; i < NR_DIR_MONITOR; i++) {//DIR
      //       mes_collect->update_pool(dir_monitors[i]->self_be_write(), i, DIR_monitor::SELF);
      //       mes_collect->update_pool(dir_monitors[i]->self_be_write_1(), i, DIR_monitor::SELF);
      //       mes_collect->update_pool(dir_monitors[i]->client_be_write(), i, DIR_monitor::CLIENT);
      //       mes_collect->update_pool(dir_monitors[i]->client_be_write_1(), i, DIR_monitor::CLIENT);
      // }
      // mes_collect->check_time_out();
      // //--------------------------//
    }

    

    if(sqr->do_reset(Cycles)){
      // send to check
      Cover::cacheState State;
      State = mes_collect->get_state_info(0x80000000);
      uint8_t cs[7];
      cs[0] = State.L1[0][0];
      cs[1] = State.L1[0][1];
      cs[2] = State.L1[1][0];
      cs[3] = State.L1[1][1];
      cs[4] = State.L2[0];
      cs[5] = State.L2[1];
      cs[6] = State.L3;
      sqr->case_with_states.check(Cycles-testcase_with_states::cycle_done, cs);

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

    if(random_mode && Cycles >= 8500) {
      for (int i = 0; i < NR_CAGENTS; i++) {
        // tl_base_agent::TLCTransaction tr = randomTest2(false, l1[i]->bus_type, ptw, dma);
        tl_base_agent::TLCTransaction tr = sqr->random_test_fullsys(sequencer::TLC, false, l1[i]->bus_type, ptw, dma);
        l1[i]->transaction_input(tr);
        // case	cycle	agent	agentid	link	operation	opcode	param	paramcode	address	uesr
        // static uint64_t link = 0;
        // HLOG(P_SW_T,"0  %ld CAgent %d %ld operation %d  param %d %lx 0\n", Cycles, i , link++, tr.opcode, tr.param, tr.addr);
      }
      // for (int i = 0; i < NR_PTWAGT; i++) {
      //   // tl_base_agent::TLCTransaction tr = randomTest3(ptw, dma, l1, ptw[i]->bus_type);
      //   tl_base_agent::TLCTransaction tr = sqr->random_test_fullsys(sequencer::TLUL, false, ptw[i]->bus_type, ptw, dma);
      //   if(tr.addr != 0x80000000)
      //     ptw[i]->transaction_input(tr);
      // }
      // for (int i = 0; i < NR_DMAAGT; i++) {
      //   // tl_base_agent::TLCTransaction tr = randomTest3(ptw, dma, l1, dma[i]->bus_type);
      //   tl_base_agent::TLCTransaction tr = sqr->random_test_fullsys(sequencer::TLUL, false, dma[i]->bus_type, ptw, dma);
      //   dma[i]->transaction_input(tr);
      // }
    }else{
      // for (int i = 0; i < NR_CAGENTS; i++) {
      //   tl_base_agent::TLCTransaction tr = sqr->case_test(sequencer::TLC, i);
      //   l1[i]->transaction_input(tr);
      // }
      // for (int i = NR_CAGENTS; i < NR_CAGENTS + NR_PTWAGT; i++) {
      //   tl_base_agent::TLCTransaction tr = sqr->case_test(sequencer::TLUL, i);
      //   ptw[i-NR_CAGENTS]->transaction_input(tr);
      // }
      // for (int i = NR_CAGENTS + NR_PTWAGT; i < NR_CAGENTS + NR_PTWAGT + NR_DMAAGT; i++) {
      //   tl_base_agent::TLCTransaction tr = sqr->case_test(sequencer::TLUL, i);
      //   dma[i-NR_CAGENTS-NR_PTWAGT]->transaction_input(tr);
      // }

      // for test with states
      for (int i = 0; i < NR_CAGENTS; i++) {
        tl_base_agent::TLCTransaction tr = sqr->case_test_with_states(sequencer::TLC, l1[i]->bus_type, l1[i]->core_id);
        l1[i]->transaction_input(tr);
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
  if(this->en_monitor){
    // report->print_report();
    sqr->case_with_states.print_report();
  }
}
