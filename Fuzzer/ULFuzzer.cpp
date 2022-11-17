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
    // if (rand() % 10) return;
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
            uint32_t mask_raw = 0;
            int offset;
            for (int i = 0; i < 4; i++) {
                mask_raw = (mask_raw << 8) + rand()%0xFF;
            }
            switch (rand() % 3) {
            case 0:
                offset = (rand() % 4) * 8;
                sent = ulAgent->do_putpartialdata(addr + offset, 3, mask_raw & (0x000000FF<<offset), genPutPartialData(putdata));
                break;
            case 1:
                offset = (rand() % 2) * 16;
                sent = ulAgent->do_putpartialdata(addr + offset, 4, mask_raw & (0x0000FFFF << offset), genPutPartialData(putdata));
                break;
            case 2:
                sent = ulAgent->do_putpartialdata(addr, 5, mask_raw & 0xFFFFFFFF, genPutPartialData(putdata));
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
