#include "Interface.h"
namespace tl_interface{
  std::shared_ptr<TLInfo> tlc_info_array[NR_CAGENTS];
  int32_t tlc_info_array_counter = 0;
  std::shared_ptr<TLInfo> tlu_info_array[NR_ULAGENTS];
  int32_t tlu_info_array_counter = 0;
  std::shared_ptr<TLInfo> monitor_info_array[NR_TL_MONITOR];
  int32_t monitor_info_array_counter = 0;

  using namespace tl_agent;
  TLInfo::TLInfo(uint64_t cid, uint8_t bt):
    id(cid), bus_type(bt)
  {
    a_opcode = 0;
    a_param = 0;
    a_size = 0;
    a_source = 0;
    a_address = 0;
    a_user = 0;
    a_echo = 0;
    a_mask = 0;
    memset(a_data, 0, BEATSIZE);
    a_user_alias = 0;
    a_user_preferCache = 0;
    a_user_needHint = 0;
    a_corrupt = 0;
    a_valid = 0;
    a_ready = 0;

    //Channel B
    b_opcode = 0;
    b_param = 0;
    b_size = 0;
    b_source = 0;
    b_address = 0;
    b_mask = 0;
    memset(b_data, 0, BEATSIZE);
    b_corrupt = 0;
    b_alias = 0;
    b_needdata = 0;
    b_valid = 0;
    b_ready = 0;

    //Channel C
    c_opcode = 0;
    c_param = 0;
    c_size = 0;
    c_source = 0;
    c_address = 0;
    c_user = 0;
    c_echo = 0;
    memset(c_data, 0, BEATSIZE);
    c_corrupt = 0;
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
    d_user = 0;
    d_echo = 0;
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

  void TLInfo::connect(std::shared_ptr<Port<ReqField, RespField, EchoField, BEATSIZE> > port){
    //Channel A
    port->a.ready     = &a_ready;
    port->a.valid     = &a_valid;
    port->a.opcode    = (uint8_t *)(&a_opcode);
    port->a.param     = (uint8_t *)(&a_param);
    port->a.address   = &a_address;
    port->a.size      = (uint8_t *)(&a_size);
    port->a.source    = &a_source;
    port->a.mask      = &a_mask;
    port->a.data      = a_data;
    port->a.alias     = (uint8_t *)(&a_user_alias);
    port->a.corrupt   = &a_corrupt;
    port->a.usr       = (tl_agent::ReqField*)&a_user;
    port->a.echo      = (tl_agent::EchoField*)&a_echo;
    //Channel B
    port->b.ready     = &b_ready;
    port->b.valid     = &b_valid;
    port->b.opcode    = (uint8_t *)&b_opcode;
    port->b.param     = (uint8_t *)(&b_param);
    port->b.address   = &b_address;
    port->b.size      = &b_size;
    port->b.source    = &b_source;
    port->b.alias     = &b_alias;
    port->b.needdata  = &b_needdata;
    port->b.corrupt   = &b_corrupt;
    //Channel C
    port->c.ready     = &c_ready;
    port->c.valid     = &c_valid;
    port->c.opcode    = (uint8_t *)(&c_opcode);
    port->c.param     = (uint8_t *)(&c_param);
    port->c.address   = &c_address;
    port->c.size      = (uint8_t *)(&c_size);
    port->c.source    = &c_source;
    port->c.data      = c_data;
    port->c.dirty     = &c_echo_blockisdirty;
    port->c.corrupt   = &c_corrupt;
    port->c.alias     = nullptr;
    port->c.usr       = (tl_agent::ReqField*)&c_user;
    port->c.echo      = (tl_agent::EchoField*)&c_echo;
    //Channel D
    port->d.ready     = &d_ready;
    port->d.valid     = &d_valid;
    port->d.opcode    = (uint8_t *)(&d_opcode);
    port->d.param     = (uint8_t *)(&d_param);
    port->d.size      = (uint8_t *)(&d_size);
    port->d.source    = &d_source;
    port->d.sink      = (uint8_t *)(&d_sink);
    port->d.denied    = &d_denied;
    port->d.dirty     = &d_echo_blockisdirty;
    port->d.data      = d_data;
    port->d.corrupt   = &d_corrupt;
    port->d.usr       = (tl_agent::RespField*)&d_user;
    port->d.echo      = (tl_agent::EchoField*)&d_echo;
    //Channel E
    port->e.ready     = &e_ready;
    port->e.valid     = &e_valid;
    port->e.sink      = (uint8_t *)(&e_sink);
    port->e.addr      = nullptr;
    port->e.alias     = nullptr;
  }

  std::shared_ptr<TLInfo> find_tlc_info(uint64_t cid, uint8_t bt){
    tlc_assert(tlc_info_array_counter != 0, "No TLC agent was created!");
    bool found = false;
    int32_t idx = tlc_info_array_counter;
    while(idx --> 0){
      if(tlc_info_array[idx]->id == cid && tlc_info_array[idx]->bus_type == bt){
        found = true;
        break;
      }
    }
    tlc_assert(found, "core ID or cache type not found!");
    return tlc_info_array[idx];
  }

  void register_tlc_info(std::shared_ptr<TLInfo> p){
    tlc_info_array[tlc_info_array_counter++] = p;
  }

  std::shared_ptr<TLInfo> find_tlu_info(uint64_t cid, uint8_t bt){
    tlc_assert(tlu_info_array_counter != 0, "No TLU agent was created!");
    bool found = false;
    int32_t idx = tlu_info_array_counter;
    while(idx --> 0){
      if(tlu_info_array[idx]->id == cid && tlu_info_array[idx]->bus_type == bt){
        found = true;
        break;
      }
    }
    tlc_assert(found, "core ID or agent type not found!");
    return tlu_info_array[idx];
  }

  void register_tlu_info(std::shared_ptr<TLInfo> p){
    tlu_info_array[tlu_info_array_counter++] = p;
  }

  std::shared_ptr<TLInfo> find_monitor_info(uint64_t id, uint8_t bt){
    tlc_assert(monitor_info_array_counter != 0, "No TLU agent was created!");
    bool found = false;
    int32_t idx = monitor_info_array_counter;
    while(idx --> 0){
      if(monitor_info_array[idx]->id == id && monitor_info_array[idx]->bus_type == bt){
        found = true;
        break;
      }
    }
    tlc_assert(found, "core ID or agent type not found!");
    return monitor_info_array[idx];
  }

  void register_monitor_info(std::shared_ptr<TLInfo> p){
    monitor_info_array[monitor_info_array_counter++] = p;
  }

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
  ){
  uint64_t cid = *(const uint64_t*)core_id;
  uint8_t  bt  = *(const uint8_t*)bus_type;
  std::shared_ptr<tl_interface::TLInfo> info = tl_interface::find_tlc_info(cid, bt);

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
  ){
    uint64_t cid = *(const uint64_t*)core_id;
    uint8_t  bt  = *(const uint8_t*)bus_type;
    std::shared_ptr<tl_interface::TLInfo> info = tl_interface::find_tlu_info(cid, bt);
    info->a_ready         = a_ready;
    *a_valid              = info->a_valid;
    *a_opcode             = info->a_opcode;
    *a_param              = info->a_param;
    *a_size               = info->a_size;
    *a_source             = info->a_source;
    *(paddr_t*)a_address  = info->a_address;
    *a_user_preferCache   = info->a_user_preferCache;
    *a_mask               = info->a_mask;
    memcpy(a_data, info->a_data, BEATSIZE);

    *d_ready              = info->d_ready;
    info->d_valid         = d_valid;
    info->d_opcode        = *d_opcode;
    info->d_size          = *d_size;
    info->d_source        = *d_source;
    info->d_denied        = d_denied;
    info->d_corrupt       = d_corrupt;
    memcpy(info->d_data, d_data, BEATSIZE);
  }

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
  ){
    uint64_t cid = *(const uint64_t*)id;
    uint8_t  bt  = *(const uint8_t*)bus_type;
    if(tl_interface::monitor_info_array_counter == 0)return;
    std::shared_ptr<tl_interface::TLInfo> info = tl_interface::find_monitor_info(cid, bt);
    //Channel A
    info->a_opcode = *a_opcode;
    info->a_param = *a_param;
    info->a_size = *a_size;
    info->a_source = *a_source;
    info->a_address = *(paddr_t *)a_address;
    info->a_user = *a_user;
    info->a_echo = *a_echo;
    info->a_mask = *a_mask;
    memcpy(info->a_data, a_data, BEATSIZE);
    info->a_corrupt = a_corrupt;
    info->a_valid = a_valid;
    info->a_ready = a_ready;

    //Channel B
    info->b_opcode = *b_opcode;
    info->b_param = *b_param;
    info->b_size = *b_size;
    info->b_source = *b_source;
    info->b_address = *(paddr_t *)b_address;
    info->b_mask = *b_mask;
    memcpy(info->b_data, b_data, BEATSIZE);
    info->b_corrupt = b_corrupt;
    info->b_valid = b_valid;
    info->b_ready = b_ready;

    //Channel C
    info->c_opcode = *c_opcode;
    info->c_param = *c_param;
    info->c_size = *c_size;
    info->c_source = *c_source;
    info->c_address = *(paddr_t *)c_address;
    info->c_user = *c_user;
    info->c_echo = *c_echo;
    memcpy(info->c_data, c_data, BEATSIZE);
    info->c_corrupt = c_corrupt;
    info->c_valid = c_valid;
    info->c_ready = c_ready;

    //Channel D
    info->d_opcode = *d_opcode;
    info->d_param = *d_param;
    info->d_size = *d_size;
    info->d_source = *d_source;
    info->d_sink = *d_sink;
    info->d_denied = d_denied;
    info->d_user = *d_user;
    info->d_echo = *d_echo;
    memcpy(info->d_data, d_data, BEATSIZE);
    info->d_corrupt = d_corrupt;
    info->d_valid = d_valid;
    info->d_ready = d_ready;

    //Channel E
    info->e_sink = *e_sink;
    info->e_valid = e_valid;
    info->e_ready = e_ready;
  }
#ifdef __cplusplus
}
#endif