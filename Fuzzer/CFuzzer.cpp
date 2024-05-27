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

inline uint16_t connect(uint8_t a, uint8_t b) {
    return (uint16_t)((a << 8) | b);
}

void CFuzzer::traceTest() {
    if (this->transactions.empty()) {
        return;
    }
    Transaction transaction = this->transactions.front();
    paddr_t addr = transaction.addr;
    uint8_t channel = transaction.channel;
    uint8_t opcode = transaction.opcode;
    uint8_t param = transaction.param;
    int send_status;

    switch (connect(channel, opcode))
    {
    case (1 << 8) | tl_agent::AcquireBlock:
        send_status = this->cAgent->do_acquireBlock(addr, param, 0);    break;
    case (1 << 8) | tl_agent::AcquirePerm:
        send_status = this->cAgent->do_acquirePerm(addr, param, 0);     break;
    // even if transaction has param, we still use releaseDataAuto here
    // in fear of releaseData may have unknown bugs untested
    case (4 << 8) | tl_agent::ReleaseData:
    case (4 << 8) | tl_agent::Release:
        send_status = this->cAgent->do_releaseDataAuto(addr, param);    break;
    default:
        std::cerr << "Error: Invalid Transaction " << channel << " Opcode " << opcode << std::endl;
        break;
    }

    // if succeeded in sending t, remove it from queue
    if (send_status == 0) {
        this->transactions.pop();
    }
    // otherwise try it next cycle
}

void CFuzzer::tick() {
    this->randomTest(true);
//    this->caseTest();
}
