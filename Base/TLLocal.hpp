#pragma once

#ifndef TLC_TEST_TLLOCAL_H
#define TLC_TEST_TLLOCAL_H

#include <cstdint>


class TLLocalContext {
public:
    virtual uint64_t        cycle() const noexcept = 0;
    virtual int             sysId() const noexcept = 0;
};


#endif // TLC_TEST_TLLOCAL_H
