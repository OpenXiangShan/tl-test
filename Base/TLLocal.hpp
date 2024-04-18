#pragma once

#ifndef TLC_TEST_TLLOCAL_H
#define TLC_TEST_TLLOCAL_H

#include <cstdint>


struct TLLocalConfig {
public:
    std::uint64_t       seed;

    unsigned int        coreCount;                          // L1-L2 system count
    unsigned int        masterCountPerCoreTLC;              // TL-C master count per core
    unsigned int        masterCountPerCoreTLUL;             // TL-UL master count per core
};


class TLLocalContext {
public:
    virtual uint64_t        cycle() const noexcept = 0;
    virtual int             sysId() const noexcept = 0;
    virtual unsigned int    sysSeed() const noexcept = 0;
};


#endif // TLC_TEST_TLLOCAL_H
