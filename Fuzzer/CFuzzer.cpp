//
// Created by wkf on 2021/10/29.
//

#include "../TLAgent/TLEnum.h"
#include "Fuzzer.h"

#include "../Events/TLSystemEvent.hpp"

#include <algorithm>


static std::vector<CFuzzRange> RANGES = {
    { .ordinal = 0, .maxTag = CFUZZER_RAND_RANGE_TAG,     .maxSet = CFUZZER_RAND_RANGE_SET,   .maxAlias = CFUZZER_RAND_RANGE_ALIAS    },
    { .ordinal = 1, .maxTag = 0x1,                        .maxSet = 0x10,                     .maxAlias = 0x4                         },
    { .ordinal = 2, .maxTag = 0x10,                       .maxSet = 0x1,                      .maxAlias = 0x4                         }
};

static inline size_t fact(size_t n) noexcept
{
    size_t r = 1;
    for (size_t i = 1; i <= n; i++)
        r *= i;
    return r;
}


CFuzzer::CFuzzer(tl_agent::CAgent *cAgent) noexcept {
    this->cAgent = cAgent;

    this->rangeIndex                = 0;
    this->rangeIterationInterval    = cAgent->config().ariInterval;
    this->rangeIterationTarget      = cAgent->config().ariTarget;

    this->rangeIterationCount       = 0;
    this->rangeIterationTime        = cAgent->config().ariInterval;

    for (size_t i = 0; i < RANGES.size(); i++)
        this->rangeOrdinal.push_back(i);

    size_t loop = cAgent->sysSeed() % fact(rangeOrdinal.size());
    for (size_t i = 0; i < loop; i++)
        std::next_permutation(rangeOrdinal.begin(), rangeOrdinal.end());

    LogInfo(this->cAgent->cycle(), Append("Initial Fuzz Set: index = ", this->rangeIndex, ", permutation: "));
    LogEx(
        std::cout << "[ ";
        for (size_t i = 0; i < rangeOrdinal.size(); i++)
            std::cout << rangeOrdinal[i] << " ";
        std::cout << "]";
    );
    LogEx(std::cout << std::endl);
}

void CFuzzer::randomTest(bool do_alias) {
    paddr_t addr = 
        ((CAGENT_RAND64(cAgent, "CFuzzer") % RANGES[rangeOrdinal[rangeIndex]].maxTag) << 13) 
      + ((CAGENT_RAND64(cAgent, "CFuzzer") % RANGES[rangeOrdinal[rangeIndex]].maxSet) << 6);  // Tag + Set + Offset
    int alias = (do_alias) ? (CAGENT_RAND64(cAgent, "CFuzzer") % RANGES[rangeOrdinal[rangeIndex]].maxAlias) : 0;
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
        cAgent->do_releaseDataAuto(addr, alias, 
            CAGENT_RAND64(cAgent, "CFuzzer") & 0x1,
            CAGENT_RAND64(cAgent, "CFuzzer") & 0x1); // feel free to releaseData according to its priv
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

    if (this->cAgent->cycle() >= this->rangeIterationTime)
    {
        this->rangeIterationTime += this->rangeIterationInterval;
        this->rangeIndex++;

        if (this->rangeIndex == rangeOrdinal.size())
        {
            this->rangeIndex = 0;
            this->rangeIterationCount++;

            std::next_permutation(rangeOrdinal.begin(), rangeOrdinal.end());
        }

        LogInfo(this->cAgent->cycle(), Append("Fuzz Set switched: index = ", this->rangeIndex, ", permutation: "));
        LogEx(
            std::cout << "[ ";
            for (size_t i = 0; i < rangeOrdinal.size(); i++)
                std::cout << rangeOrdinal[i] << " ";
            std::cout << "]";
        );
        LogEx(std::cout << std::endl);
    }

    if (this->rangeIterationCount == this->rangeIterationTarget)
    {
        TLSystemFinishEvent().Fire();
    }
}
