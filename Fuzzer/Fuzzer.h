//
// Created by wkf on 2021/10/29.
//

#ifndef TLC_TEST_FUZZER_H
#define TLC_TEST_FUZZER_H

#include "../TLAgent/ULAgent.h"
#include "../TLAgent/CAgent.h"
#include "../Utils/Common.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <queue>

class Fuzzer {
protected:
    uint64_t *cycles;
public:
    Fuzzer() = default;
    ~Fuzzer() = default;
    // virtual void tick(bool mode, std::string line) = 0;
    virtual void tick() = 0;
    virtual int transaction(int channel, int opcode, paddr_t address, int param) = 0;
    void set_cycles(uint64_t *cycles) {
        this->cycles = cycles;
    }
};

class ULFuzzer: public Fuzzer {
private:
    tl_agent::ULAgent *ulAgent;
public:
    ULFuzzer(tl_agent::ULAgent *ulAgent);
    void randomTest(bool put);
    void caseTest();
    void caseTest2();
    int transaction(int channel, int opcode, paddr_t address, int param);
    // void tick(bool mode, std::string line);
    void tick();
};

class CFuzzer: public Fuzzer {
private:
    tl_agent::CAgent *cAgent;
public:
    CFuzzer(tl_agent::CAgent *cAgent);
    void randomTest(bool do_alias);
    void caseTest();
    // void traceTest(std::string line);
    int transaction(int channel, int opcode, paddr_t address, int param);
    // void tick(bool mode, std::string line);
    void tick();
};




#endif //TLC_TEST_FUZZER_H
