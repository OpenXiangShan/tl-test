//
// Created by wkf on 2021/10/29.
//

#include "Fuzzer.h"

ULFuzzer::ULFuzzer(tl_agent::ULAgent *ulAgent) {
    this->ulAgent = ulAgent;
}

uint8_t* genPutData(uint8_t* putdata) {
    for (int i = 0; i < DATASIZE; i++) {
        putdata[i] = (uint8_t)rand();
    }
    return putdata;
}

uint8_t* genPutPartialData(uint8_t* putdata) {
    for (int i = 0; i < DATASIZE/2; i++) {
        putdata[i] = (uint8_t)rand();
    }
    for (int i = DATASIZE/2; i < DATASIZE; i++) {
        putdata[i] = putdata[i-DATASIZE/2];
    }
    return putdata;
}

void ULFuzzer::randomTest(tl_agent::BaseAgent ** agent) {
    // address generation
    bool sent = false;
    paddr_t addr;
    bool flag;
    uint8_t* putdata = new uint8_t[DATASIZE];
    for (int i = 0; i < 10; i++) {
        addr = (rand() % 0x8) * (rand() % 0x80) * 0x40; // Tag + Set + Offset
        flag = true;
        // probe all agents including itself
        for (int j = 0; j < NR_CAGENTS+NR_ULAGENTS; j++) {
            if (agent[j]->local_probe(addr)) {
                flag = false;
                break;
            }
        }
        if (flag) break;
    }
    if (!flag) return;

    if (rand() % 2) {  // Get
        ulAgent->do_getAuto(addr);
    } else {
        if (rand() % 2) {  // PutFullData
            sent = ulAgent->do_putfulldata(addr, genPutData(putdata));
        } else {  // PutPartialData
          switch (rand() % 3) {
          case 0:
              sent = ulAgent->do_putpartialdata(addr, 3, 0x000000FF << (rand()%4*8), genPutPartialData(putdata));
              break;
          case 1:
              sent = ulAgent->do_putpartialdata(addr, 4, 0x0000FFFF << (rand()%2*16), genPutPartialData(putdata));
              break;
          case 2:
              sent = ulAgent->do_putpartialdata(addr, 5, 0xFFFFFFFF, genPutPartialData(putdata));
              break;
          }
        }
    }
    if (!sent)
        delete[](putdata);
}

void ULFuzzer::caseTest() {
    uint8_t* putdata = new uint8_t[DATASIZE];
    if (*cycles == 500) {
        ulAgent->do_putpartialdata(0x1070, 2, 0xf0000, genPutPartialData(putdata));
    }
    if (*cycles == 600) {
        ulAgent->do_getAuto(0x1040);
    }
}

void ULFuzzer::caseTest2() {
    uint8_t* putdata = new uint8_t[DATASIZE];
    if (*cycles == 100) {
        ulAgent->do_putpartialdata(0x1000, 2, 0xf, genPutPartialData(putdata));
    }
    if (*cycles == 500) {
        ulAgent->do_get(0x1000, 2, 0xf);
    }
}

void ULFuzzer::tick(tl_agent::BaseAgent ** agent) {
    this->randomTest(agent);
//    this->caseTest();
}
