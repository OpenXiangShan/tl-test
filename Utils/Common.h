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
  NR_TILE_MONITOR = 2,
  NR_L3_MONITOR = 4,
  NR_DMA_MONITOR = 1,
  NR_TL_MONITOR = NR_TILE_MONITOR + NR_L3_MONITOR + NR_DMA_MONITOR,
  NR_DIR_L3_MONITOR = 4,
  NR_DIR_L2_core0_MONITOR = 4,
  NR_DIR_L2_core1_MONITOR = 4,
  NR_DIR_MONITOR = NR_DIR_L2_core0_MONITOR + NR_DIR_L2_core1_MONITOR + NR_DIR_L3_MONITOR,
  TIMEOUT_INTERVAL = 10000,
  DRAM_OFFSET = 0x80000000
};
enum {
  DCACHE_BUS_TYPE = 0,
  ICACHE_BUS_TYPE = 1,
  TILE_BUS_TYPE = 2,
  L3_BUS_TYPE = 3,
  DMA_BUS_TYPE = 4,
  PTW_BUS_TYPE = 5,
  AGENT_BUS_TYPE_MAX = 6,
  DIR_BUS_TYPE = 6,
  DCACHE_A_SOURCE_BEGIN = 0,
  DCACHE_A_SOURCE_END = 16,
  DCACHE_C_SOURCE_BEGIN = 16,
  DCACHE_C_SOURCE_END = 34,
  ICACHE_A_SOURCE_BEGIN = 0,
  ICACHE_A_SOURCE_END = 4,
  ICACHE_C_SOURCE_BEGIN = 4,
  ICACHE_C_SOURCE_END = 5,
  PTW_A_SOURCE_BEGIN = 0,
  PTW_A_SOURCE_END = 7,
  DMA_A_SOURCE_BEGIN = 0,
  DMA_A_SOURCE_END = 1 << 9,
};
enum {
  CHNLA = 0,
  CHNLB,
  CHNLC,
  CHNLD,
  CHNLE,
  LEVEL_L1L2 = 0,
  LEVEL_L2L3,
  LEVEL_L3MEM,
};
//mes type
enum{
    REQ = 0,
    ACK = 1,
    ACK1 = 2,
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
      printf("%s: ",this->type_to_string().c_str());                           \
      printf(__VA_ARGS__);                                                     \
      fflush(stdout);                                                          \
    }                                                                          \
  } while (0)

#define Dump(...)                                                              \
  do {                                                                         \
    if (Verbose) {                                                             \
      printf(__VA_ARGS__);                                                     \
    }                                                                          \
  } while (0)

#define GET_CA_A_ID_BEGIN(ct)                                                   \
  ((ct == DCACHE_BUS_TYPE)? DCACHE_A_SOURCE_BEGIN:ICACHE_A_SOURCE_BEGIN)

#define GET_CA_A_ID_END(ct)                                                   \
  ((ct == DCACHE_BUS_TYPE)? DCACHE_A_SOURCE_END:ICACHE_A_SOURCE_END) 

#define GET_CA_C_ID_BEGIN(ct)                                                   \
  ((ct == DCACHE_BUS_TYPE)? DCACHE_C_SOURCE_BEGIN:ICACHE_C_SOURCE_BEGIN)

#define GET_CA_C_ID_END(ct)                                                   \
  ((ct == DCACHE_BUS_TYPE)? DCACHE_C_SOURCE_END:ICACHE_C_SOURCE_END) 

#define GET_UA_A_ID_BEGIN(ct)                                                   \
  ((ct == PTW_BUS_TYPE)? PTW_A_SOURCE_BEGIN:DMA_A_SOURCE_BEGIN)

#define GET_UA_A_ID_END(ct)                                                   \
  ((ct == PTW_BUS_TYPE)? PTW_A_SOURCE_END:DMA_A_SOURCE_END)    

/*
#define Log(...) \
    do {} while(0)

#define Dump(...) \
    do {} while(0)
*/

#endif // TLC_TEST_COMMON_H
