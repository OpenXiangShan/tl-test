//
// Created by wkf on 2021/10/29.
//

#include "Fuzzer.h"

ULFuzzer::ULFuzzer(tl_agent::ULAgent *ulAgent) {
    this->ulAgent = ulAgent;
    srand((unsigned)time(0));
}

void ULFuzzer::tick() {
    paddr_t addr = (rand() % 10) * 0x100;
    if (rand() % 2) {  // Get
        ulAgent->do_get(addr);
    } else { // Put
        uint8_t* putdata = new uint8_t[DATASIZE];
        for (int i = 0; i < DATASIZE; i++) {
            putdata[i] = (uint8_t)rand();
        }
        ulAgent->do_putfulldata(addr, putdata);
    }
}