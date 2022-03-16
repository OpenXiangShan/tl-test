//
// Created by wkf on 2021/10/29.
//

#include "Fuzzer.h"

ULFuzzer::ULFuzzer(tl_agent::ULAgent *ulAgent) {
    this->ulAgent = ulAgent;
}

void ULFuzzer::randomTest() {
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

void ULFuzzer::caseTest() {
    if (*cycles == 500) {
        uint8_t* putdata = new uint8_t[DATASIZE];
        for (int i = 0; i < DATASIZE/2; i++) {
            putdata[i] = (uint8_t)rand();
        }
        for (int i = DATASIZE/2; i < DATASIZE; i++) {
            putdata[i] = putdata[i-DATASIZE/2];
        }
        this->ulAgent->do_putpartialdata(0x1000, 4, 0xffff, putdata);
    }
}

bool ULFuzzer::tick() {
    this->randomTest();
//    this->caseTest();
    return true;    
}