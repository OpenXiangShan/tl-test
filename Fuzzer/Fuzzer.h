//
// Created by wkf on 2021/10/29.
//

#ifndef TLC_TEST_FUZZER_H
#define TLC_TEST_FUZZER_H

#include "../TLAgent/ULAgent.h"
#include "../TLAgent/CAgent.h"

#include <vector>


class Fuzzer {
protected:
    uint64_t *cycles;
public:
    Fuzzer() noexcept = default;
    virtual ~Fuzzer() noexcept = default;
    virtual void tick() = 0;
    inline void set_cycles(uint64_t *cycles) {
        this->cycles = cycles;
    }
};

class ULFuzzer: public Fuzzer {
private:
    tl_agent::ULAgent *ulAgent;
public:
    ULFuzzer(tl_agent::ULAgent *ulAgent) noexcept;
    virtual ~ULFuzzer() noexcept = default;
    void randomTest(bool put);
    void caseTest();
    void caseTest2();
    void tick();
};


struct CFuzzRange {
    size_t      ordinal;
    uint64_t    maxTag;
    uint64_t    maxSet;
    uint64_t    maxAlias;

    inline bool operator<(const CFuzzRange& obj) const noexcept
    {
        return ordinal < obj.ordinal;
    }

    inline void swap(CFuzzRange& obj) const noexcept
    {

    }
};

class CFuzzer: public Fuzzer {
private:
    tl_agent::CAgent*   cAgent;
    size_t              rangeIndex;
    size_t              rangeIterateTime;
    size_t              rangeIteration;
    std::vector<int>    rangeOrdinal;
public:
    CFuzzer(tl_agent::CAgent *cAgent) noexcept;
    virtual ~CFuzzer() noexcept = default;
    void randomTest(bool do_alias);
    void caseTest();
    void tick();
};

#endif //TLC_TEST_FUZZER_H
