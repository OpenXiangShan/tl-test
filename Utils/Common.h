//
// Created by Kaifan Wang on 2021/10/27.
//

#ifndef TLC_TEST_COMMON_H
#define TLC_TEST_COMMON_H

#include <cmath>
#include <memory>
#include <string>

extern uint64_t Cycles;
extern bool Verbose;

enum {
  DATASIZE = 64, // Cache line is 64B
  BEATSIZE = 32,
  NR_CAGENTS = 4,
  NR_SOURCEID = 16,
  NR_PTWAGT = 2,
  NR_DMAAGT = 1,
  NR_ULAGENTS = NR_PTWAGT + NR_DMAAGT,
  TIMEOUT_INTERVAL = 5000,
  DRAM_OFFSET = 0x80000000
};
enum {
  DCACHE_TYPE = 0, 
  ICACHE_TYPE = 1,
  PTW_TYPE = 0,
  DMA_TYPE = 1,
  DCACHE_SOURCE_WD = 6,
  ICACHE_SOURCE_WD = 3,
  PTW_SOURCE_WD = 3,
  DMA_SOURCE_WD = 9
};

const uint64_t FULLMASK = 0xFFFFFFFFFFFFFFFF;
typedef uint64_t paddr_t;

#define tlc_assert(cond, info)                                                 \
  do {                                                                         \
    if (!(cond)) {                                                             \
      printf("\33[1;34m%s\n", info);                                           \
      printf("Cycles: %ld\33[0m\n", Cycles);                                   \
      fflush(stdout);                                                          \
      fflush(stderr);                                                          \
      throw(1);                                                            \
    }                                                                          \
  } while (0)

#define Log(...)                                                               \
  do {                                                                         \
    if (Verbose) {                                                             \
      printf("%s: ",this->type_to_string().c_str());                               \
      printf(__VA_ARGS__);                                                     \
    }                                                                          \
  } while (0)

#define Dump(...)                                                              \
  do {                                                                         \
    if (Verbose) {                                                             \
      printf(__VA_ARGS__);                                                     \
    }                                                                          \
  } while (0)

#define GET_CA_ID_UPBOUND(ct)                                                   \
  (((ct == DCACHE_TYPE)? (1 << DCACHE_SOURCE_WD):(1 << ICACHE_SOURCE_WD)))      

#define GET_UA_ID_UPBOUND(ct)                                                   \
  (((ct == PTW_TYPE)? (1 << PTW_SOURCE_WD):(1 << DMA_SOURCE_WD)))      

/*
#define Log(...) \
    do {} while(0)

#define Dump(...) \
    do {} while(0)
*/

#endif // TLC_TEST_COMMON_H
