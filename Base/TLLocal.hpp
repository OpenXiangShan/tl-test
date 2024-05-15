#pragma once

#ifndef TLC_TEST_TLLOCAL_H
#define TLC_TEST_TLLOCAL_H

#include <cstdint>


struct TLLocalConfig {
public:
    uint64_t            seed;

    unsigned int        coreCount;                          // L1-L2 system count
    unsigned int        masterCountPerCoreTLC;              // TL-C master count per core
    unsigned int        masterCountPerCoreTLUL;             // TL-UL master count per core

    uint64_t            ariInterval;                        // Auto Range Iteration interval
    uint64_t            ariTarget;                          // Auto Range Iteration target
};


class TLLocalContext {
public:
    virtual uint64_t                cycle() const noexcept = 0;
    virtual int                     sysId() const noexcept = 0;
    virtual unsigned int            sysSeed() const noexcept = 0;

    virtual const TLLocalConfig&    config() const noexcept = 0;
    virtual TLLocalConfig&          config() noexcept = 0;
};


#endif // TLC_TEST_TLLOCAL_H
