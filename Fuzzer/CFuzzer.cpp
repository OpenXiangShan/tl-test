//
// Created by wkf on 2021/10/29.
//

#include "../TLAgent/TLEnum.h"
#include "Fuzzer.h"


#define CFUZZER_RAND_RANGE_TAG              0x4
#define CFUZZER_RAND_RANGE_SET              0x4
#define CFUZZER_RAND_RANGE_ALIAS            0x4


#ifndef CFUZZER_RAND_RANGE_TAG
#   define CFUZZER_RAND_RANGE_TAG           0x8
#endif

#ifndef CFUZZER_RAND_RANGE_SET
#   define CFUZZER_RAND_RANGE_SET           0x80
#endif

#ifndef CFUZZER_RAND_RANGE_ALIAS
#   define CFUZZER_RAND_RANGE_ALIAS         0x4
#endif


CFuzzer::CFuzzer(tl_agent::CAgent *cAgent) noexcept {
    this->cAgent = cAgent;
}

void CFuzzer::randomTest(bool do_alias) {
    paddr_t addr = 
        ((CAGENT_RAND64(cAgent, "CFuzzer") % CFUZZER_RAND_RANGE_TAG) << 13) 
      + ((CAGENT_RAND64(cAgent, "CFuzzer") % CFUZZER_RAND_RANGE_SET) << 6);  // Tag + Set + Offset
    int alias = (do_alias) ? (CAGENT_RAND64(cAgent, "CFuzzer") % CFUZZER_RAND_RANGE_ALIAS) : 0;
    if (CAGENT_RAND64(cAgent, "CFuzzer") % 2) {
        if (CAGENT_RAND64(cAgent, "CFuzzer") % 3) {
            if (CAGENT_RAND64(cAgent, "CFuzzer") % 2) {
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
            putdata[i] = (uint8_t)CAGENT_RAND64(cAgent, "CFuzzer");
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
        auto putdata = make_shared_tldata<DATASIZE>();
        for (int i = 0; i < DATASIZE; i++) {
            putdata->data[i] = (uint8_t)CAGENT_RAND64(cAgent, "CFuzzer");
        }
        this->cAgent->do_releaseData(0x1040, tl_agent::TtoN, putdata, 0);
    }
    if (*cycles == 400) {
      this->cAgent->do_acquireBlock(0x1040, tl_agent::NtoT, 0);
    }
}

void CFuzzer::tick() {
    this->randomTest(true);
//    this->caseTest();
}
