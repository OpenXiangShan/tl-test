//
// Created by wkf on 2021/10/29.
//

#include "Fuzzer.h"

CFuzzer::CFuzzer(tl_agent::CAgent *cAgent) {
    this->cAgent = cAgent;
    addr_ifstream.open("/nfs-nvme/home/zhouyaoyang/projects/ff-reshape/system.cpu.dcache.tags.hex.txt");
}

void CFuzzer::randomTest() {
    paddr_t addr = (rand() % 0x100) * 0x100;
    if (rand() % 2) {
        if (rand() % 3) {
            cAgent->do_acquireBlock(addr, tl_agent::NtoT); // AcquireBlock
        } else {
            cAgent->do_acquirePerm(addr, tl_agent::NtoT);  // AcquirePerm
        }
    } else {
        uint8_t* putdata = new uint8_t[DATASIZE];
        for (int i = 0; i < DATASIZE; i++) {
            putdata[i] = (uint8_t)rand();
        }
        cAgent->do_releaseData(addr, tl_agent::TtoN, putdata); // ReleaseData
    }
}

void CFuzzer::caseTest() {
    if (*cycles == 100) {
        this->cAgent->do_acquireBlock(0x1000, tl_agent::NtoT);
    }
    if (*cycles == 300) {
        uint8_t* putdata = new uint8_t[DATASIZE];
        for (int i = 0; i < DATASIZE; i++) {
            putdata[i] = (uint8_t)rand();
        }
        this->cAgent->do_releaseData(0x1000, tl_agent::TtoN, putdata);
    }
}

void CFuzzer::tick() {
    // this->randomTest();
    this->warmupTraffic();
}

void CFuzzer::warmupTraffic() {
    if (last_block_addr == 0) {
        addr_ifstream >> std::hex >> last_block_addr;
        std::cout << "Address:" << std::hex << last_block_addr << std::endl;
    }
    if (this->cAgent->do_acquireBlock(last_block_addr, tl_agent::NtoT)) {
        // clear it to indicate sent
        last_block_addr = 0;
    }
}