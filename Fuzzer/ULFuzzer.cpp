//
// Created by wkf on 2021/10/29.
//

#include "Fuzzer.h"

ULFuzzer::ULFuzzer(tl_agent::ULAgent *ulAgent) noexcept {
    this->ulAgent = ulAgent;
}

void ULFuzzer::randomTest(bool put) {
    paddr_t addr = (CAGENT_RAND64(ulAgent, "CFuzzer") % 0x400) << 6;
    if (!put || CAGENT_RAND64(ulAgent, "CFuzzer") % 2) {  // Get
        ulAgent->do_getAuto(addr);
    } else { // Put
        auto putdata = make_shared_tldata<DATASIZE>();
        for (int i = 0; i < DATASIZE; i++) {
            putdata->data[i] = (uint8_t)CAGENT_RAND64(ulAgent, "CFuzzer");
        }
        ulAgent->do_putfulldata(addr, putdata);
    }
}

void ULFuzzer::caseTest() {
    if (*cycles == 500) {
        auto putdata = make_shared_tldata<DATASIZE>();
        for (int i = 0; i < DATASIZE/2; i++) {
            putdata->data[i] = (uint8_t)CAGENT_RAND64(ulAgent, "CFuzzer");
        }
        for (int i = DATASIZE/2; i < DATASIZE; i++) {
            putdata->data[i] = putdata->data[i-DATASIZE/2];
        }
        ulAgent->do_putpartialdata(0x1070, 2, 0xf0000, putdata);
    }
    if (*cycles == 600) {
      ulAgent->do_getAuto(0x1040);
    }
}

void ULFuzzer::caseTest2() {
  if (*cycles == 100) {
    auto putdata = make_shared_tldata<DATASIZE>();
    for (int i = 0; i < DATASIZE/2; i++) {
      putdata->data[i] = (uint8_t)CAGENT_RAND64(ulAgent, "CFuzzer");
    }
    for (int i = DATASIZE/2; i < DATASIZE; i++) {
      putdata->data[i] = putdata->data[i-DATASIZE/2];
    }
    ulAgent->do_putpartialdata(0x1000, 2, 0xf, putdata);
  }
  if (*cycles == 500) {
    ulAgent->do_get(0x1000, 2, 0xf);
  }
}

void ULFuzzer::tick() {
    this->randomTest(false);
//    this->caseTest();
}
