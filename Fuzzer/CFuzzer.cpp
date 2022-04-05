//
// Created by wkf on 2021/10/29.
//

#include "Fuzzer.h"

CFuzzer::CFuzzer(tl_agent::CAgent *cAgent) {
    this->cAgent = cAgent;
}

void CFuzzer::randomTest() {
    paddr_t addr = (rand() % 0x8) * (rand() % 0x80) * 0x40;  // Tag + Set + Offset
    if (rand() % 2) {
        if (rand() % 3) {
            if (rand() % 2) {
                cAgent->do_acquireBlock(addr, tl_agent::NtoT, rand() % 4); // AcquireBlock NtoT
            } else {
                cAgent->do_acquireBlock(addr, tl_agent::NtoT, rand() % 4); // AcquireBlock NtoB
            }
        } else {
            cAgent->do_acquirePerm(addr, tl_agent::NtoT, rand() % 4); // AcquirePerm
        }
    } else {
        /*
        uint8_t* putdata = new uint8_t[DATASIZE];
        for (int i = 0; i < DATASIZE; i++) {
            putdata[i] = (uint8_t)rand();
        }
        cAgent->do_releaseData(addr, tl_agent::TtoN, putdata); // ReleaseData
        */
        cAgent->do_releaseDataAuto(addr, rand() % 4); // feel free to releaseData according to its priv
    }
}

void CFuzzer::caseTest() {
    if (*cycles == 100) {
        this->cAgent->do_acquireBlock(0x1000, tl_agent::NtoT, rand() % 4);
    }
    if (*cycles == 300) {
        uint8_t* putdata = new uint8_t[DATASIZE];
        for (int i = 0; i < DATASIZE; i++) {
            putdata[i] = (uint8_t)rand();
        }
        this->cAgent->do_releaseData(0x1000, tl_agent::TtoN, putdata, rand() % 4);
    }
}

void CFuzzer::tick() {
    this->randomTest();
//    this->caseTest();
}