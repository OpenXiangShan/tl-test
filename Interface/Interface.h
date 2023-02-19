#ifndef TLC_TEST_INTERFACE_H
#define TLC_TEST_INTERFACE_H
#include "svdpi.h"
#include "../Utils/Common.h"
#include "../TLAgent/Port.h"
#include "../TLAgent/BaseAgent.h"
#include <string.h>

namespace tl_interface{
  using namespace tl_agent;
  class TLInfo{
    public:
    std::shared_ptr<uint64_t>             id;
    std::shared_ptr<uint8_t>              bus_type;

    //Channel A
    std::shared_ptr<uint8_t>              a_opcode;
    std::shared_ptr<uint8_t>              a_param;
    std::shared_ptr<uint8_t>              a_size;
    std::shared_ptr<uint32_t>             a_source;
    std::shared_ptr<paddr_t>              a_address;
    std::shared_ptr<tl_agent::ReqField>   a_user;
    std::shared_ptr<tl_agent::EchoField>  a_echo;
    std::shared_ptr<uint32_t>             a_mask;
    std::shared_ptr<uint8_t[]>            a_data;
    std::shared_ptr<uint8_t>              a_user_alias;
    std::shared_ptr<uint8_t>              a_user_preferCache;
    std::shared_ptr<uint8_t>              a_user_needHint;
    std::shared_ptr<uint8_t>              a_corrupt;
    std::shared_ptr<uint8_t>              a_valid;
    std::shared_ptr<uint8_t>              a_ready;

    //Channel B
    std::shared_ptr<uint8_t>              b_opcode;
    std::shared_ptr<uint8_t>              b_param;
    std::shared_ptr<uint8_t>              b_size;
    std::shared_ptr<uint32_t>             b_source;
    std::shared_ptr<paddr_t>              b_address;
    std::shared_ptr<uint32_t>             b_mask;
    std::shared_ptr<uint8_t[]>            b_data;
    std::shared_ptr<uint8_t>              b_corrupt;
    std::shared_ptr<uint8_t>              b_alias;
    std::shared_ptr<uint8_t>              b_needdata;
    std::shared_ptr<uint8_t>              b_valid;
    std::shared_ptr<uint8_t>              b_ready;

    //Channel C
    std::shared_ptr<uint8_t>              c_opcode;
    std::shared_ptr<uint8_t>              c_param;
    std::shared_ptr<uint8_t>              c_size;
    std::shared_ptr<uint32_t>             c_source;
    std::shared_ptr<paddr_t>              c_address;
    std::shared_ptr<tl_agent::ReqField>   c_user;
    std::shared_ptr<tl_agent::EchoField>  c_echo;
    std::shared_ptr<uint8_t[]>            c_data;
    std::shared_ptr<uint8_t>              c_corrupt;
    std::shared_ptr<uint8_t>              c_echo_blockisdirty;
    std::shared_ptr<uint8_t>              c_valid;
    std::shared_ptr<uint8_t>              c_ready;

    //Channel D
    std::shared_ptr<uint8_t>              d_opcode;
    std::shared_ptr<uint8_t>              d_param;
    std::shared_ptr<uint8_t>              d_size;
    std::shared_ptr<uint32_t>             d_source;
    std::shared_ptr<uint32_t>             d_sink;
    std::shared_ptr<uint8_t>              d_denied;
    std::shared_ptr<tl_agent::RespField>  d_user;
    std::shared_ptr<tl_agent::EchoField>  d_echo;
    std::shared_ptr<uint8_t[]>            d_data;
    std::shared_ptr<uint8_t>              d_corrupt;
    std::shared_ptr<uint8_t>              d_echo_blockisdirty;
    std::shared_ptr<uint8_t>              d_valid;
    std::shared_ptr<uint8_t>              d_ready;

    //Channel E
    std::shared_ptr<uint32_t>             e_sink;
    std::shared_ptr<uint8_t>              e_valid;
    std::shared_ptr<uint8_t>              e_ready;

    TLInfo(uint64_t cid, uint8_t ct);
    void connect(std::shared_ptr<Port<ReqField, RespField, EchoField, BEATSIZE> > port);
  };

  extern std::shared_ptr<TLInfo> tlc_info_array[NR_CAGENTS];
  extern int32_t tlc_info_array_counter;
  std::shared_ptr<TLInfo> find_tlc_info(uint64_t cid, uint8_t ct);
  void register_tlc_info(std::shared_ptr<TLInfo>);

  extern std::shared_ptr<TLInfo> tlu_info_array[NR_ULAGENTS];
  extern int32_t tlu_info_array_counter;
  std::shared_ptr<TLInfo> find_tlu_info(uint64_t cid, uint8_t at);
  void register_tlu_info(std::shared_ptr<TLInfo>);
  
  extern std::shared_ptr<TLInfo> monitor_info_array[NR_TL_MONITOR];
  extern int32_t monitor_info_array_counter;
  std::shared_ptr<TLInfo> find_monitor_info(uint64_t id, uint8_t bt);
  void register_monitor_info(std::shared_ptr<TLInfo> p);
}

#ifdef __cplusplus
extern "C" {
#endif
void tlc_agent_eval (
  const svBitVecVal*    core_id,
  const svBitVecVal*    bus_type,
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

void tlu_agent_eval(
  const svBitVecVal*    core_id,
  const svBitVecVal*    bus_type,         
  svBit                 a_ready,
  svBit*                a_valid,
  svBitVecVal*          a_opcode,
  svBitVecVal*          a_param,
  svBitVecVal*          a_size,
  svBitVecVal*          a_source,
  svBitVecVal*          a_address,
  svBit*                a_user_preferCache,
  svBitVecVal*          a_mask,
  svBitVecVal*          a_data,
  svBit*                d_ready,
  svBit                 d_valid,
  const svBitVecVal*    d_opcode,
  const svBitVecVal*    d_size,
  const svBitVecVal*    d_source,
  svBit                 d_denied,
  const svBitVecVal*    d_data,
  svBit                 d_corrupt
);


void tlc_monitor_eval(
  const svBitVecVal*    id,
  const svBitVecVal*    bus_type,
    //Channel A
  const svBitVecVal*    a_opcode,
  const svBitVecVal*    a_param,
  const svBitVecVal*    a_size,
  const svBitVecVal*    a_source,
  const svBitVecVal*    a_address,
  const svBitVecVal*    a_user,
  const svBitVecVal*    a_echo,
  const svBitVecVal*    a_mask,
  const svBitVecVal*    a_data,
  svBit                 a_corrupt,
  svBit                 a_valid,
  svBit                 a_ready,

    //Channel B
  const svBitVecVal*    b_opcode,
  const svBitVecVal*    b_param,
  const svBitVecVal*    b_size,
  const svBitVecVal*    b_source,
  const svBitVecVal*    b_address,
  const svBitVecVal*    b_mask,
  const svBitVecVal*    b_data,
  svBit                 b_corrupt,
  svBit                 b_valid,
  svBit                 b_ready,

    //Channel C
  const svBitVecVal*    c_opcode,
  const svBitVecVal*    c_param,
  const svBitVecVal*    c_size,
  const svBitVecVal*    c_source,
  const svBitVecVal*    c_address,
  const svBitVecVal*    c_user,
  const svBitVecVal*    c_echo,
  const svBitVecVal*    c_data,
  svBit                 c_corrupt,
  svBit                 c_valid,
  svBit                 c_ready,

    //Channel D
  const svBitVecVal*    d_opcode,
  const svBitVecVal*    d_param,
  const svBitVecVal*    d_size,
  const svBitVecVal*    d_source,
  const svBitVecVal*    d_sink,
  svBit                 d_denied,
  const svBitVecVal*    d_user,
  const svBitVecVal*    d_echo,
  const svBitVecVal*    d_data,
  svBit                 d_corrupt,
  svBit                 d_valid,
  svBit                 d_ready,

    //Channel E
  const svBitVecVal*    e_sink,
  svBit                 e_valid,
  svBit                 e_ready
  );
#ifdef __cplusplus
}
#endif

#endif //TLC_TEST_INTERFACE_H