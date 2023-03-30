//
// Created by ljw on 10/22/21.
//

#ifndef TLC_TEST_EMU_H
#define TLC_TEST_EMU_H

#include "Vtb_top.h"
#include "verilated.h"
#include <getopt.h>
#if VM_TRACE == 1
#include "verilated_vcd_c.h"
#include "verilated_fst_c.h"
#endif
#include "../Fuzzer/Fuzzer.h"
#include "../TLAgent/CAgent.h"
#include "../TLAgent/ULAgent.h"
#include "../Utils/Common.h"
#include "../Utils/ScoreBoard.h"
#include "../Monitor/Monitor.h"
#include "../CacheModel/FakeL1/fake_l1.h"
#include "../CacheModel/FakePTW/fake_ptw.h"
#include "../Sequencer/sequencer.h"
#include "../Monitor/DIR_Monitor.h"
#include "../Cover/MesCollect.h"
#include "../Cover/MesCom.h"
#include "../Cover/Report.h"


class Emu {
private:
  typedef tl_agent::BaseAgent BaseAgent_t;
  typedef tl_agent::ULAgent ULAgent_t;
  typedef tl_agent::CAgent CAgent_t;
  typedef tl_monitor::Monitor Monitor_t;
  typedef DIR_monitor::DIR_Monitor DIR_Monitor_t;
  typedef DIR_monitor::Dir_key Dir_key_t;
  typedef DIR_monitor::Dir_Mes Dir_Mes_t;
  
  typedef fake_l1::FakeL1 FakeL1_t;
  typedef fake_ptw::FakePTW FakePTW_t;
  typedef fake_ptw::FakePTW FakeDMA_t;
  typedef sequencer::Sequencer Sequencer_t;

  typedef Cover::Mes_Collect Mes_Collect_t;
  typedef Cover::Mes_Com Mes_Com_t;
  typedef Cover::Report Report_t;

  const static int NR_AGENTS = NR_CAGENTS + NR_ULAGENTS;
  Vtb_top *dut_ptr;
  // VerilatedVcdC *tfp;
  VerilatedFstC *tfp;
  GlobalBoard<paddr_t> *globalBoard;
  std::shared_ptr<Monitor_t>monitors[NR_TL_MONITOR];
  std::shared_ptr<DIR_Monitor_t>dir_monitors[NR_DIR_MONITOR];
  ScoreBoard<Dir_key_t,Dir_Mes_t> selfDir[3];
  ScoreBoard<Dir_key_t,paddr_t> selfTag[3];
  ScoreBoard<Dir_key_t,Dir_Mes_t> clientDir[3];
  ScoreBoard<Dir_key_t,paddr_t> clientTag[3];
              

  uint64_t seed = 0, wave_begin = 0, wave_end = 0;
  bool en_monitor = false;
  bool enable_wave = true;
  bool wave_full = false;
  bool all_wave = false;
  bool random_mode = true;
  inline char *cycle_wavefile(uint64_t cycles, time_t t);
  void parse_args(int argc, char **argv);

  std::shared_ptr<FakeL1_t> l1[NR_CAGENTS];
  std::shared_ptr<FakePTW_t> ptw[NR_PTWAGT];
  std::shared_ptr<FakeDMA_t> dma[NR_DMAAGT];
  std::shared_ptr<Sequencer_t> sqr;

  std::shared_ptr<Mes_Collect_t> mes_collect;
  Mes_Com_t *mes_com;
  Report_t *report;


public:
  Emu(int argc, char **argv);
  ~Emu();
  uint64_t exe_cycles = 10000000;
  inline void reset(uint64_t n);
  inline void neg_edge();
  inline void pos_edge();
  inline void step();
  inline void update_cycles(uint64_t inc);
  void execute(uint64_t nr_cycle);
  void reset_sys(uint64_t n);//reset system, include tl-test environment
};

inline void Emu::reset(uint64_t n) {
  for (uint64_t i = 0; i < n; i++) {
    dut_ptr->reset = 1;
    dut_ptr->clock = 0;
    dut_ptr->eval();
    dut_ptr->clock = 1;
    dut_ptr->eval();
  }
  printf("reset is done!\n");
  fflush(stdout);
  dut_ptr->reset = 0;
}

inline void Emu::neg_edge() {
  dut_ptr->clock = 0;
  dut_ptr->eval();
#if VM_TRACE == 1
  if (this->enable_wave && Cycles >= this->wave_begin && Cycles <= this->wave_end) {
    this->tfp->dump((vluint64_t)Cycles * 2);
  }
#endif
}

inline void Emu::pos_edge() {
  dut_ptr->clock = 1;
  dut_ptr->eval();
#if VM_TRACE == 1
  if (this->enable_wave && Cycles >= this->wave_begin && Cycles <= this->wave_end) {
    this->tfp->dump((vluint64_t)Cycles * 2 + 1);
  }
#endif
}

inline void Emu::step(){
  neg_edge();
  pos_edge();
}

inline void Emu::update_cycles(uint64_t inc) {
  Cycles += inc;
  if (Cycles % 100000 == 0) {
    printf("%lu cycles have passed!\n", Cycles);
  }
}

inline char *Emu::cycle_wavefile(uint64_t cycles, time_t t) {
  static char buf[1024];
  char buf_time[64];
  strftime(buf_time, sizeof(buf_time), "%F@%T", localtime(&t));
  char *pwd = getcwd(NULL, 0);
  tlc_assert(pwd != NULL, "Wavefile cannot be opened!");
  int len = snprintf(buf, 1024, "%s/%s_%lu", pwd, buf_time, cycles);
  // strcpy(buf + len, ".vcd");
  strcpy(buf + len, ".fst");
  printf("dump wave to %s...\n", buf);
  return buf;
}

#endif // TLC_TEST_EMU_H
