//
// Created by wkf on 2021/10/29.
//

#include "Fuzzer.h"

CFuzzer::CFuzzer(std::shared_ptr<tl_agent::CAgent>cAgent) { this->cAgent = cAgent; }

void CFuzzer::randomTest(bool do_alias, std::shared_ptr<tl_agent::BaseAgent> *agent) {
  // if (rand() % 10) return;
  // address generation
  paddr_t addr;
  bool flag;
  for (int i = 0; i < 10; i++) {
    addr = (rand() % 0x8) * (rand() % 0x80) * 0x40 + DRAM_OFFSET; // Tag + Set + Offset
    flag = true;
    // probe all ul-agents
    for (int j = NR_CAGENTS; j < NR_CAGENTS + NR_ULAGENTS; j++) {
      if (agent[j]->local_probe(addr)) {
        flag = false;
        break;
      }
    }
    if (flag)
      break;
  }
  if (!flag)
    return;

  int alias = (do_alias) ? (rand() % 4) : 0;
  uint8_t param = cAgent->bus_type == ICACHE_BUS_TYPE? tl_agent::NtoB:tl_agent::NtoT;
  if (rand() % 2) {
    if (rand() % 3) {
      if (rand() % 2) {
        cAgent->do_acquireBlock(addr, param,
                                alias); // AcquireBlock NtoT
      } else {
        cAgent->do_acquireBlock(addr, param,
                                alias); // AcquireBlock NtoB
      }
    } else {
      if(cAgent->bus_type == DCACHE_BUS_TYPE)cAgent->do_acquirePerm(addr, param, alias); // AcquirePerm
    }
  } else {
    cAgent->do_releaseDataAuto(
        addr, alias); // feel free to releaseData according to its priv
  }
}

//TODO : perfercache can be set
void CFuzzer::caseTest(int id) {
  if(case_mes[id].haskey(*cycles) == true)
  {
      // std::cout << "agent:" << id << " haskey" << std::endl;
      //Case Message
      testcase::Mes_Entry mes = case_mes[id].query(*cycles);
      //genAddr
      static paddr_t addr = 0x80000000;
      if(mes.addr == 0)
        addr = addr;
      else if(mes.addr == 1)
        addr = genAddr();
      else
        addr = mes.addr;

      switch(mes.opcode){
          case tl_agent::AcquireBlock :
              this->cAgent->do_acquireBlock(addr, mes.param, mes.user);
              break;
          case tl_agent::AcquirePerm :
              this->cAgent->do_acquirePerm(addr, mes.param, mes.user);
              break;
          case (tl_agent::ReleaseData+1) :{//因为releasdata与acquireperm的param重复，+1以视为区分
                  std::shared_ptr<uint8_t[]>putdata(new uint8_t[DATASIZE]);
                  for (int i = 0; i < DATASIZE; i++) {
                    putdata[i] = (uint8_t)rand();
                  }
                  this->cAgent->do_releaseData(addr, mes.param, putdata, mes.user);
                  break;
              }
          case testcase::reset_opcode : break;
          default:printf("caseTest error\n");
      }
  }
}

void CFuzzer::tick(std::shared_ptr<tl_agent::BaseAgent> *agent, int id, int mode) {
  if(mode == true)
    this->randomTest(false, agent);
  else
    this->caseTest(id);
}
