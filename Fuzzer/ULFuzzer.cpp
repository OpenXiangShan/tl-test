//
// Created by wkf on 2021/10/29.
//

#include "Fuzzer.h"

ULFuzzer::ULFuzzer(tl_agent::ULAgent *ulAgent) {
    this->ulAgent = ulAgent;
}

void ULFuzzer::tick() {
    if (*this->cycles == 100) {
        ulAgent->do_get(0x1000);
    }
    if (*cycles == 200) {
        uint8_t* putdata = new uint8_t[DATASIZE];
        for (int i = 0; i < DATASIZE; i++) {
            putdata[i] = (uint8_t)rand();
        }
        ulAgent->do_putfulldata(0x2000, putdata);
    }
    if (*cycles == 202) {
        ulAgent->do_get(0x2000);
    }
}