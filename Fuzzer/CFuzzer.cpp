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
  if (rand() % 2) {
    if (rand() % 3) {
      if (rand() % 2) {
        cAgent->do_acquireBlock(addr, tl_agent::NtoT,
                                alias); // AcquireBlock NtoT
      } else {
        cAgent->do_acquireBlock(addr, tl_agent::NtoT,
                                alias); // AcquireBlock NtoB
      }
    } else {
      cAgent->do_acquirePerm(addr, tl_agent::NtoT, alias); // AcquirePerm
    }
  } else {
    cAgent->do_releaseDataAuto(
        addr, alias); // feel free to releaseData according to its priv
  }
}

void CFuzzer::caseTest() {
  if (*cycles == 100) {
    this->cAgent->do_acquireBlock(0x1040, tl_agent::NtoT, 0);
  }
  if (*cycles == 300) {
    uint8_t *putdata = new uint8_t[DATASIZE];
    for (int i = 0; i < DATASIZE; i++) {
      putdata[i] = (uint8_t)rand();
    }
    this->cAgent->do_releaseData(0x1040, tl_agent::TtoN, putdata, 0);
  }
  if (*cycles == 400) {
    this->cAgent->do_acquireBlock(0x1040, tl_agent::NtoT, 0);
  }
}

void CFuzzer::tick(std::shared_ptr<tl_agent::BaseAgent> *agent) {
  this->randomTest(false, agent);
  //    this->caseTest();
}
