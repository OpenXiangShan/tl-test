//
// Created by wkf on 2021/10/29.
//

#ifndef TLC_TEST_FUZZER_H
#define TLC_TEST_FUZZER_H

#include "../TLAgent/ULAgent.h"
#include "../TLAgent/CAgent.h"

#include <iostream>
#include <fstream>

class Fuzzer {
protected:
    uint64_t *cycles;
public:
    Fuzzer() = default;
    ~Fuzzer() = default;
    virtual bool tick() = 0;
    void set_cycles(uint64_t *cycles) {
        this->cycles = cycles;
    }
};

class ULFuzzer: public Fuzzer {
private:
    tl_agent::ULAgent *ulAgent;
public:
    ULFuzzer(tl_agent::ULAgent *ulAgent);
    void randomTest();
    void caseTest();
    bool tick() override;
};

class CFuzzer: public Fuzzer {
private:
    tl_agent::CAgent *cAgent;
    uint64_t last_block_addr = 0;
    std::ifstream addr_ifstream;
public:
    CFuzzer(tl_agent::CAgent *cAgent);
    void randomTest();
    void caseTest();
    void warmupTraffic();
    bool tick() override;
    bool stream_end() {return addr_ifstream.peek() == EOF;}
};

#endif //TLC_TEST_FUZZER_H
