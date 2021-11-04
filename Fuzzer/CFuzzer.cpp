//
// Created by wkf on 2021/10/29.
//

#include "Fuzzer.h"

CFuzzer::CFuzzer(tl_agent::CAgent *cAgent) {
    this->cAgent = cAgent;
    srand((unsigned)time(0));
}

void CFuzzer::tick() {
    if (*cycles == 200) {
        this->cAgent->do_acquireBlock(0x1000, tl_agent::NtoB);
    }
    if (*cycles == 201) {
        this->cAgent->do_acquireBlock(0x2000, tl_agent::NtoT);
    }
}