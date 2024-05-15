//
// Created by wkf on 2021/10/29.
//

#include "Fuzzer.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <queue>

// // for 36-bit address(same as xiangshan)
// paddr_t fullAddr(unsigned tag, unsigned set, unsigned bank, unsigned offset = 0) {
//     tag = tag % 0x10000;
//     set = set % 0x1000;
//     bank = bank % 0x4;
//     return (paddr_t)((tag << 20) + (set << 8) + (bank << 6) + offset);
// }

// // TODO: alias
// struct Transaction
// {
//     uint64_t timestamp;
//     int sender;
//     int channel;
//     int opcode;
//     paddr_t address;
//     int param;

//     Transaction(){}
//     Transaction(uint64_t timestamp, int sender, int channel, int opcode, paddr_t address, int param):
//         timestamp(timestamp), sender(sender), channel(channel), opcode(opcode), address(address), param(param) {}

//     void parseManual(std::string line) {
//         std::stringstream ss(line);
//         std::string value;
//         std::vector<std::string> values;

//         while (std::getline(ss, value, ',')) {
//             values.push_back(value);
//         }
//         // timestamp, which L1, channel, opcode, tag, set, bank, param
//         this->timestamp = std::stoull(values[0]);
//         this->sender = std::stoi(values[1]);
//         this->channel = std::stoi(values[2]);
//         this->opcode = std::stoi(values[3]);
//         this->param = std::stoi(values[4]);
//         unsigned tag = std::stoul(values[5]);
//         unsigned set = std::stoul(values[6]);
//         unsigned bank = std::stoul(values[7]);
//         this->address = fullAddr(tag, set, bank);

//     }
//     void parseDB(std::string path) {
//     }
// };

CFuzzer::CFuzzer(tl_agent::CAgent *cAgent) {
    this->cAgent = cAgent;
}

void CFuzzer::randomTest(bool do_alias) {
    paddr_t addr = ((rand() % 0x2000) << 20) + ((rand() % 0x4000) << 6);
    int alias = (do_alias) ? (rand() % 4) : 0;
    if (rand() % 2) {
        if (rand() % 3) {
            if (rand() % 2) {
                // printf("AcquireBlock NtoT, addr = %016x\n", addr);  
                cAgent->do_acquireBlock(addr, tl_agent::NtoT, alias); // AcquireBlock NtoT
            } else {
                // printf("AcquireBlock NtoB, addr = %016x\n", addr);  
                cAgent->do_acquireBlock(addr, tl_agent::NtoB, alias); // AcquireBlock NtoB
            }
        } else {
            // printf("AcquirePerm NtoT, addr = %016x\n", addr);  
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
        // printf("ReleaseData, addr = %016x\n", addr);  
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

// void CFuzzer::traceTest(std::string line) {
//     const static int NR_AGENTS = NR_CAGENTS + NR_ULAGENTS;
//     Fuzzer ** const fuzzers = new Fuzzer*[NR_AGENTS];
//     Transaction t = Transaction();
//     t.parseManual(line);

//     int code = fuzzers[t.sender]->transaction(t.channel, t.opcode, t.address, t.param);
//     if(code) {
//         printf("L1_%d Failed to send transaction: %s, by %d\n", t.sender, line.c_str(), code);
//         // TODO: should retry when failed
//         assert(0);
//     }
// }

// bool CFuzzer::transaction(int channel, int opcode, paddr_t address, int param) {
int CFuzzer::transaction(int channel, int opcode, paddr_t address, int param) {
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


// void CFuzzer::tick(bool mode, std::string line) {
//     if (mode) {
//         this->traceTest(line);
//     } else {
//         this->randomTest(true);
//     }
// }

void CFuzzer::tick() {
    this->randomTest(false);
}
