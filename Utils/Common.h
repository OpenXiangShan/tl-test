//
// Created by Kaifan Wang on 2021/10/27.
//

#ifndef TLC_TEST_COMMON_H
#define TLC_TEST_COMMON_H

enum {
    DATASIZE = 64, // Cache line is 64B
    NR_ULAGENTS = 1,
    NR_CAGENTS = 0,
    NR_SOURCEID = 16
};

#include <cmath>
#include <string>
#include <assert.h>

#define tlc_assert(cond, info) \
    do { \
        if (!(cond)) { \
            printf("\33[1;34m%s\33[0m\n", info); \
            assert(cond); \
        } \
    } while (0)

#endif //TLC_TEST_COMMON_H
