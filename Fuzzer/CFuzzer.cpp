//
// Created by wkf on 2021/10/29.
//

#include "Fuzzer.h"

CFuzzer::CFuzzer(tl_agent::CAgent *cAgent) {
    this->cAgent = cAgent;
}

void CFuzzer::randomTest(bool do_alias) {
    paddr_t addr = ((rand() % 0x8) << 13) + ((rand() % 0x80) << 6);  // Tag + Set + Offset
    int alias = (do_alias) ? (rand() % 4) : 0;
    if (rand() % 2) {
        if (rand() % 3) {
            if (rand() % 2) {
                cAgent->do_acquireBlock(addr, tl_agent::NtoT, alias); // AcquireBlock NtoT
            } else {
                cAgent->do_acquireBlock(addr, tl_agent::NtoB, alias); // AcquireBlock NtoB
            }
        } else {
            cAgent->do_acquirePerm(addr, tl_agent::NtoT, alias); // AcquirePerm
        }
    } else {
        /*
        uint8_t* putdata = new uint8_t[DATASIZE];
        for (int i = 0; i < DATASIZE; i++) {
            putdata[i] = (uint8_t)rand();
        }
        cAgent->do_releaseData(addr, tl_agent::TtoN, putdata); // ReleaseData
        */
        cAgent->do_releaseDataAuto(addr, alias); // feel free to releaseData according to its priv
    }
}

void CFuzzer::caseTest() {
    if (*cycles == 100) {
        this->cAgent->do_acquireBlock(0x1040, tl_agent::NtoT, 0);
    }
    if (*cycles == 300) {
        uint8_t* putdata = new uint8_t[DATASIZE];
        for (int i = 0; i < DATASIZE; i++) {
            putdata[i] = (uint8_t)rand();
        }
        this->cAgent->do_releaseData(0x1040, tl_agent::TtoN, putdata, 0);
    }
    if (*cycles == 400) {
      this->cAgent->do_acquireBlock(0x1040, tl_agent::NtoT, 0);
    }
}

bool CFuzzer::transaction(int channel, int opcode, paddr_t address, int param) {
    switch (channel) {
        case 1:
            switch (opcode) {
                case 6:
                    // printf("CFuzzer: Acquire: 0x%x %d %d %d\n", address, channel, opcode, param);
                    return this->cAgent->do_acquireBlock(address, param, 0);
                case 7:
                    return this->cAgent->do_acquirePerm(address, param, 0);
                default:
                    return false;
            }
        case 4:
            switch (opcode) {
                case 6:
                case 7:
                    return this->cAgent->do_releaseDataAuto(address, 0);
                default:
                    return false;
            }
        default:
            return false;
    }
    return false;
}


void CFuzzer::tick() {
    this->randomTest(true);
//    this->caseTest();
}
