#ifndef TLC_TEST_INTERFACE_H
#define TLC_TEST_INTERFACE_H
#include "svdpi.h"
#include "../Utils/Common.h"
#include "../TLAgent/Port.h"
#include "../TLAgent/BaseAgent.h"
#include <string.h>

namespace tl_interface{
  using namespace tl_agent;
  class TLCInfo{
    public:
    uint64_t    core_id;
    uint8_t     cache_type;
    //Channel A
    uint32_t    a_opcode;
    uint32_t    a_param;
    uint32_t    a_size;
    uint32_t    a_source;
    uint64_t    a_address;
    uint32_t    a_mask;
    uint32_t    a_user_alias;
    uint8_t     a_user_preferCache;
    uint8_t     a_user_needHint;
    uint8_t     a_valid;
    uint8_t     a_ready;

    //Channel B
    uint32_t    b_param;
    uint64_t    b_address;
    uint8_t     b_data[BEATSIZE];
    uint8_t     b_alias;
    uint8_t     b_needdata;
    uint8_t     b_valid;
    uint8_t     b_ready;

    //Channel C
    uint32_t    c_opcode;
    uint32_t    c_param;
    uint32_t    c_size;
    uint32_t    c_source;
    uint64_t    c_address;
    uint8_t     c_data[BEATSIZE];
    uint8_t     c_echo_blockisdirty;
    uint8_t     c_valid;
    uint8_t     c_ready;

    //Channel D
    uint32_t    d_opcode;
    uint32_t    d_param;
    uint32_t    d_size;
    uint32_t    d_source;
    uint32_t    d_sink;
    uint8_t     d_denied;
    uint8_t     d_data[BEATSIZE];
    uint8_t     d_corrupt;
    uint8_t     d_echo_blockisdirty;
    uint8_t     d_valid;
    uint8_t     d_ready;

    //Channel E
    uint32_t    e_sink;
    uint8_t     e_valid;
    uint8_t     e_ready;

    TLCInfo(uint64_t cid, uint8_t ct);
    void connect(std::shared_ptr<Port<ReqField, RespField, EchoField, BEATSIZE> > port);
  };

  extern std::shared_ptr<TLCInfo> tlc_info_array[NR_CAGENTS];
  extern int32_t tlc_info_array_counter;
  std::shared_ptr<TLCInfo> find_tlc_info(uint64_t cid, uint8_t ct);
  void register_tlc_info(std::shared_ptr<TLCInfo>);
}

#ifdef __cplusplus
extern "C" {
#endif
void tlc_agent_eval (
  const svBitVecVal*    core_id,
  svBit                 cache_type,
  //Channel A
  svBitVecVal*          a_opcode,
  svBitVecVal*          a_param,
  svBitVecVal*          a_size,
  svBitVecVal*          a_source,
  svBitVecVal*          a_address,
  svBitVecVal*          a_mask,
  svBitVecVal*          a_user_alias,
  svBit*                a_user_preferCache,
  svBit*                a_user_needHint,
  svBit*                a_valid,
  svBit                 a_ready,

  //Channel B
  const svBitVecVal*    b_param,
  const svBitVecVal*    b_address,
  const svBitVecVal*    b_data,
  svBit                 b_valid,
  svBit*                b_ready,

  //Channel C
  svBitVecVal*          c_opcode,
  svBitVecVal*          c_param,
  svBitVecVal*          c_size,
  svBitVecVal*          c_source,
  svBitVecVal*          c_address,
  svBitVecVal*          c_data,
  svBit*                c_echo_blockisdirty,
  svBit*                c_valid,
  svBit                 c_ready,

  //Channel D
  const svBitVecVal*    d_opcode,
  const svBitVecVal*    d_param,
  const svBitVecVal*    d_size,
  const svBitVecVal*    d_source,
  const svBitVecVal*    d_sink,
  svBit                 d_denied,
  const svBitVecVal*    d_data,
  svBit                 d_corrupt,
  svBit                 d_echo_blockisdirty,
  svBit                 d_valid,
  svBit*                d_ready,

  //Channel E
  svBitVecVal*          e_sink,
  svBit*                e_valid,
  svBit                 e_ready
  );
#ifdef __cplusplus
}
#endif

#endif //TLC_TEST_INTERFACE_H