//
// Created by Kaifan Wang on 2021/10/27.
//

#ifndef TLC_TEST_COMMON_H
#define TLC_TEST_COMMON_H

enum {
    DATASIZE = 64, // Cache line is 64B
    BEATSIZE = 32,
    NR_ULAGENTS = 0,
    NR_CAGENTS = 1,
    NR_SOURCEID = 16,
    TIMEOUT_INTERVAL = 1000
};

#include <memory>
#include <cmath>
#include <string>
#include <assert.h>

typedef uint16_t paddr_t;

#define tlc_assert(cond, info) \
    do { \
        if (!(cond)) { \
            printf("\33[1;34m%s\33[0m\n", info); \
            assert(cond); \
        } \
    } while (0)

#define Log(...) \
 do { \
   printf(__VA_ARGS__); \
 } while(0)


#endif //TLC_TEST_COMMON_H
