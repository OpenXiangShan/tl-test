//
// Created by wkf on 2021/10/29.
//

#ifndef TLC_TEST_FUZZER_H
#define TLC_TEST_FUZZER_H

#include "../TLAgent/ULAgent.h"
#include "../TLAgent/CAgent.h"

class Fuzzer {
protected:
    uint64_t *cycles;
public:
    Fuzzer() = default;
    ~Fuzzer() = default;
    virtual void tick(tl_agent::BaseAgent ** agent) = 0;
    void set_cycles(uint64_t *cycles) {
        this->cycles = cycles;
    }
};

class ULFuzzer: public Fuzzer {
private:
    tl_agent::ULAgent *ulAgent;
public:
    ULFuzzer(tl_agent::ULAgent *ulAgent);
    void randomTest(tl_agent::BaseAgent ** agent);
    void caseTest();
    void caseTest2();
    void tick(tl_agent::BaseAgent ** agent);
};

class CFuzzer: public Fuzzer {
private:
    tl_agent::CAgent *cAgent;
public:
    CFuzzer(tl_agent::CAgent *cAgent);
    void randomTest(bool do_alias, tl_agent::BaseAgent ** agent);
    void caseTest();
    void tick(tl_agent::BaseAgent ** agent);
};

#endif //TLC_TEST_FUZZER_H
