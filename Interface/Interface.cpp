#include "Interface.h"
namespace tl_interface{
  using namespace tl_agent;
  TLCInfo::TLCInfo(uint64_t cid, uint8_t ct):
    core_id(cid), cache_type(ct)
  {
    tlc_info_array[tlc_info_array_counter++].reset(this);
    a_opcode = 0;
    a_param = 0;
    a_size = 0;
    a_source = 0;
    a_address = 0;
    a_mask = 0;
    a_user_alias = 0;
    a_user_preferCache = 0;
    a_user_needHint = 0;
    a_valid = 0;
    a_ready = 0;

    //Channel B
    b_param = 0;
    b_address = 0;
    memset(b_data, 0, BEATSIZE);
    b_valid = 0;
    b_ready = 0;

    //Channel C
    c_opcode = 0;
    c_param = 0;
    c_size = 0;
    c_source = 0;
    c_address = 0;
    memset(c_data, 0, BEATSIZE);
    c_echo_blockisdirty = 0;
    c_valid = 0;
    c_ready = 0;

    //Channel D
    d_opcode = 0;
    d_param = 0;
    d_size = 0;
    d_source = 0;
    d_sink = 0;
    d_denied = 0;
    memset(d_data, 0, BEATSIZE);
    d_corrupt = 0;
    d_echo_blockisdirty = 0;
    d_valid = 0;
    d_ready = 0;

    //Channel E
    e_sink = 0;
    e_valid = 0;
    e_ready = 0;
  }

  void TLCInfo::connect(std::shared_ptr<Port<ReqField, RespField, EchoField, BEATSIZE> > port){
    //Channel A
    port->a.ready     = &a_ready;
    port->a.valid     = &a_valid;
    port->a.opcode    = (uint8_t *)(&a_opcode);
    port->a.param     = (uint8_t *)(&a_param);
    port->a.address   = &a_address;
    port->a.size      = (uint8_t *)(&a_size);
    port->a.source    = (uint8_t *)(&a_source);
    port->a.mask      = &a_mask;
    port->a.data      = nullptr;
    port->a.alias     = (uint8_t *)(&a_user_alias);
    port->a.corrupt   = nullptr;
    port->a.usr       = nullptr;
    port->a.echo        = nullptr;
    //Channel B
    port->b.ready     = &b_ready;
    port->b.valid     = &b_valid;
    port->b.opcode    = nullptr;
    port->b.param     = (uint8_t *)(&b_param);
    port->b.address   = &b_address;
    port->b.size      = nullptr;
    port->b.source    = nullptr;
    port->b.alias     = &b_alias;
    port->b.needdata  = &b_needdata;
    port->b.corrupt   = nullptr;
    //Channel C
    port->c.ready     = &c_ready;
    port->c.valid     = &c_valid;
    port->c.opcode    = (uint8_t *)(&c_opcode);
    port->c.param     = (uint8_t *)(&c_param);
    port->c.address   = &c_address;
    port->c.size      = (uint8_t *)(&c_size);
    port->c.source    = (uint8_t *)(&c_source);
    port->c.data      = c_data;
    port->c.dirty     = &c_echo_blockisdirty;
    port->c.corrupt   = nullptr;
    port->c.alias     = nullptr;
    port->c.usr       = nullptr;
    port->c.echo      = nullptr;
    //Channel D
    port->d.ready     = &d_ready;
    port->d.valid     = &d_valid;
    port->d.opcode    = (uint8_t *)(&d_opcode);
    port->d.param     = (uint8_t *)(&d_param);
    port->d.size      = (uint8_t *)(&d_size);
    port->d.source    = (uint8_t *)(&d_source);
    port->d.sink      = (uint8_t *)(&d_sink);
    port->d.denied    = &d_denied;
    port->d.dirty     = &d_echo_blockisdirty;
    port->d.data      = d_data;
    port->d.corrupt   = &d_corrupt;
    port->d.usr       = nullptr;
    port->d.echo      = nullptr;
    //Channel E
    port->e.ready     = &e_ready;
    port->e.valid     = &e_valid;
    port->e.sink      = (uint8_t *)(&e_sink);
    port->e.addr      = nullptr;
    port->e.alias     = nullptr;
  }

  std::shared_ptr<TLCInfo> find_tlc_info(uint64_t cid, uint8_t ct){
    tlc_assert(tlc_info_array_counter != 0, "No TLC agent was created!");
    bool found = false;
    int32_t idx = tlc_info_array_counter;
    while(idx --> 0){
      if(tlc_info_array[idx]->core_id == cid && tlc_info_array[idx]->cache_type == ct){
        found = true;
        break;
      }
    }
    tlc_assert(found, "core ID or cache type not found!");
    return tlc_info_array[idx];
  }

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
  ){
  uint64_t cid = *core_id;
  uint8_t  ct  = cache_type;
  std::shared_ptr<tl_interface::TLCInfo> info = tl_interface::find_tlc_info(cid, ct);

  //Channel A
  *a_opcode                   = info->a_opcode;
  *a_param                    = info->a_param;
  *a_size                     = info->a_size;
  *a_source                   = info->a_source;
  *(uint64_t*)a_address       = info->a_address;
  *a_mask                     = info->a_mask;
  *a_user_alias               = info->a_user_alias;
  *a_user_preferCache         = info->a_user_preferCache;
  *a_user_needHint            = info->a_user_needHint;
  *a_valid                    = info->a_valid;
  info->a_ready               = a_ready;

  //Channel B
  info->b_param               = *b_param;
  info->b_address             = *(const uint64_t*)b_address;
  memcpy(info->b_data, b_data, BEATSIZE);
  info->b_alias               = info->b_data[0] & 0x1;
  info->b_needdata            = (info->b_data[0] & 0xe) >> 1;
  info->b_valid               = b_valid;
  *b_ready                    = info->b_ready;

  //Channel C
  *c_opcode                   = info->c_opcode;
  *c_param                    = info->c_param;
  *c_size                     = info->c_size;
  *c_source                   = info->c_source;
  *(uint64_t*)c_address       = info->c_address;
  memcpy(c_data, info->c_data, BEATSIZE);
  *c_echo_blockisdirty        = info->c_echo_blockisdirty;
  *c_valid                    = info->c_valid;
  info->c_ready               = c_ready;

  //Channel D
  info->d_opcode              = *d_opcode;
  info->d_param               = *d_param;
  info->d_size                = *d_size;
  info->d_source              = *d_source;
  info->d_sink                = *d_sink;
  info->d_denied              = d_denied;
  memcpy(info->d_data, d_data, BEATSIZE);
  info->d_corrupt             = d_corrupt;
  info->d_echo_blockisdirty   = d_echo_blockisdirty;
  info->d_valid               = d_valid;
  *d_ready                    = info->d_ready;

  //Channel E
  *e_sink                     = info->e_sink;
  *e_valid                    = info->e_valid;
  info->e_ready               = e_ready;
}
#ifdef __cplusplus
}
#endif