#include "Interface.h"
namespace tl_interface{
  std::shared_ptr<TLInfo> tlc_info_array[NR_CAGENTS];
  int32_t tlc_info_array_counter = 0;
  std::shared_ptr<TLInfo> tlu_info_array[NR_ULAGENTS];
  int32_t tlu_info_array_counter = 0;
  std::shared_ptr<TLInfo> monitor_info_array[NR_TL_MONITOR];
  int32_t monitor_info_array_counter = 0;
  //dir
  std::shared_ptr<DIRInfo> dir_monitor_info_array[NR_DIR_MONITOR];
  int32_t dir_monitor_info_array_counter = 0;

  using namespace tl_agent;
  TLInfo::TLInfo(uint64_t cid, uint8_t bt)
  {
    id.reset(new uint64_t(cid));
    bus_type.reset(new uint8_t(bt));

    a_opcode.reset(new uint8_t(0));
    a_param.reset(new uint8_t(0));
    a_size.reset(new uint8_t(0));
    a_source.reset(new uint32_t(0));
    a_address.reset(new paddr_t(0));
    a_user.reset(new tl_agent::ReqField());
    a_echo.reset(new tl_agent::EchoField());
    a_mask.reset(new uint32_t(0));
    a_data.reset(new uint8_t[BEATSIZE]);
    memset(a_data.get(), 0, BEATSIZE);
    a_user_alias.reset(new uint8_t(0));
    a_user_preferCache.reset(new uint8_t(0));
    a_user_needHint.reset(new uint8_t(0));
    a_corrupt.reset(new uint8_t(0));
    a_valid.reset(new uint8_t(0));
    a_ready.reset(new uint8_t(0));

    //Channel B
    b_opcode.reset(new uint8_t(6));
    b_param.reset(new uint8_t(0));
    b_size.reset(new uint8_t(ceil(log2((double)DATASIZE))));
    b_source.reset(new uint32_t(0));
    b_address.reset(new paddr_t(0));
    b_mask.reset(new uint32_t(0xFFFFFFFF));
    b_data.reset(new uint8_t[BEATSIZE]);
    memset(b_data.get(), 0, BEATSIZE);
    b_corrupt.reset(new uint8_t(0));
    b_alias.reset(new uint8_t(0));
    b_needdata.reset(new uint8_t(0));
    b_valid.reset(new uint8_t(0));
    b_ready.reset(new uint8_t(1));

    //Channel C
    c_opcode.reset(new uint8_t(0));
    c_param.reset(new uint8_t(0));
    c_size.reset(new uint8_t(ceil(log2((double)DATASIZE))));
    c_source.reset(new uint32_t(0));
    c_address.reset(new paddr_t(0));
    c_user.reset(new tl_agent::ReqField());
    c_echo.reset(new tl_agent::EchoField());
    c_data.reset(new uint8_t[BEATSIZE]);
    memset(c_data.get(), 0, BEATSIZE);
    c_corrupt.reset(new uint8_t(0));
    c_echo_blockisdirty.reset(new uint8_t(0));
    c_valid.reset(new uint8_t(0));
    c_ready.reset(new uint8_t(0));

    //Channel D
    d_opcode.reset(new uint8_t(0));
    d_param.reset(new uint8_t(0));
    d_size.reset(new uint8_t(0));
    d_source.reset(new uint32_t(0));
    d_sink.reset(new uint32_t(0));
    d_denied.reset(new uint8_t(0));
    d_user.reset(new tl_agent::RespField());
    d_echo.reset(new tl_agent::EchoField());
    d_data.reset(new uint8_t[BEATSIZE]);
    memset(d_data.get(), 0, BEATSIZE);
    d_corrupt.reset(new uint8_t(0));
    d_echo_blockisdirty.reset(new uint8_t(0));
    d_valid.reset(new uint8_t(0));
    d_ready.reset(new uint8_t(1));

    //Channel E
    e_sink.reset(new uint32_t(0));
    e_valid.reset(new uint8_t(0));
    e_ready.reset(new uint8_t(0));
  }

  void TLInfo::connect(std::shared_ptr<Port<ReqField, RespField, EchoField, BEATSIZE> > port){
    //Channel A
    port->a.ready    = a_ready; 
    port->a.valid    = a_valid; 
    port->a.opcode   = a_opcode; 
    port->a.param    = a_param; 
    port->a.address  = a_address; 
    port->a.size     = a_size; 
    port->a.source   = a_source; 
    port->a.mask     = a_mask; 
    port->a.data     = a_data; 
    port->a.alias    = a_user_alias; 
    port->a.corrupt  = a_corrupt; 
    port->a.usr      = a_user; 
    port->a.echo     = a_echo; 
    port->a.preferCache = a_user_preferCache;
    //Channel B
    port->b.ready    = b_ready; 
    port->b.valid    = b_valid; 
    port->b.opcode   = b_opcode; 
    port->b.param    = b_param; 
    port->b.address  = b_address; 
    port->b.size     = b_size; 
    port->b.source   = b_source; 
    port->b.alias    = b_alias; 
    port->b.needdata = b_needdata; 
    port->b.corrupt  = b_corrupt; 
    //Channel C
    port->c.ready    = c_ready; 
    port->c.valid    = c_valid; 
    port->c.opcode   = c_opcode; 
    port->c.param    = c_param; 
    port->c.address  = c_address; 
    port->c.size     = c_size; 
    port->c.source   = c_source; 
    port->c.data     = c_data; 
    port->c.dirty    = c_echo_blockisdirty; 
    port->c.corrupt  = c_corrupt; 
    port->c.alias    = nullptr;
    port->c.usr      = c_user; 
    port->c.echo     = c_echo; 
    //Channel D
    port->d.ready    = d_ready; 
    port->d.valid    = d_valid; 
    port->d.opcode   = d_opcode; 
    port->d.param    = d_param; 
    port->d.size     = d_size; 
    port->d.source   = d_source; 
    port->d.sink     = d_sink; 
    port->d.denied   = d_denied; 
    port->d.dirty    = d_echo_blockisdirty; 
    port->d.data     = d_data; 
    port->d.corrupt  = d_corrupt; 
    port->d.usr      = d_user; 
    port->d.echo     = d_echo; 
    //Channel E
    port->e.ready    = e_ready; 
    port->e.valid    = e_valid; 
    port->e.sink     = e_sink; 
    port->e.addr     = nullptr;
    port->e.alias    = nullptr;
  }

  DIRInfo::DIRInfo(uint64_t cid, uint8_t bt)
  {
    id.reset(new uint64_t(cid));
    bus_type.reset(new uint8_t(bt));

    dirWReq_ready.reset(new uint8_t(1));//ready
    dirWReq_valid.reset(new uint8_t(0));//valid
    dirWReq_bits_set.reset(new paddr_t(0));//addr
    dirWReq_bits_way.reset(new uint8_t(0));//addr
    dirWReq_bits_data_dirty.reset(new uint8_t(0));
    dirWReq_bits_data_state.reset(new uint8_t(0));
    dirWReq_bits_data_clientStates_0.reset(new uint8_t(0));
    dirWReq_bits_data_clientStates_1.reset(new uint8_t(0));
    dirWReq_bits_data_prefetch.reset(new uint8_t(0));

    tagWReq_ready.reset(new uint8_t(1));//ready
    tagWReq_valid.reset(new uint8_t(0));//valid
    tagWReq_bits_set.reset(new paddr_t(0));//addr
    tagWReq_bits_way.reset(new uint8_t(0));//addr
    tagWReq_bits_tag.reset(new paddr_t(0));//addr
    
    clientDirWReq_ready.reset(new uint8_t(1));//ready
    clientDirWReq_valid.reset(new uint8_t(0));//valid
    clientDirWReq_bits_set.reset(new paddr_t(0));//addr
    clientDirWReq_bits_way.reset(new uint8_t(0));//addr
    clientDirWReq_bits_data_0_state.reset(new uint8_t(0));
    clientDirWReq_bits_data_0_alias.reset(new uint8_t(0));
    clientDirWReq_bits_data_1_state.reset(new uint8_t(0));
    clientDirWReq_bits_data_1_alias.reset(new uint8_t(0));
    
    clientTagWreq_ready.reset(new uint8_t(1));//ready
    clientTagWreq_valid.reset(new uint8_t(0));//valid
    clientTagWreq_bits_set.reset(new paddr_t(0));//addr
    clientTagWreq_bits_way.reset(new uint8_t(0));//addr
    clientTagWreq_bits_tag.reset(new paddr_t(0));//addr
  }

  std::shared_ptr<TLInfo> find_tlc_info(uint64_t cid, uint8_t bt){
    tlc_assert(tlc_info_array_counter != 0, "No TLC agent was created!");
    bool found = false;
    int32_t idx = tlc_info_array_counter;
    while(idx --> 0){
      if(*(tlc_info_array[idx]->id) == cid && *(tlc_info_array[idx]->bus_type) == bt){
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
      if(*(tlu_info_array[idx]->id) == cid && *(tlu_info_array[idx]->bus_type) == bt){
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
      if(*(monitor_info_array[idx]->id) == id && *(monitor_info_array[idx]->bus_type) == bt){
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

  //DIRInfo
  std::shared_ptr<DIRInfo> find_dir_monitor_info(uint64_t id, uint8_t bt){
    tlc_assert(dir_monitor_info_array_counter != 0, "No DIR Monitor was created!");
    bool found = false;
    int32_t idx = dir_monitor_info_array_counter;
    while(idx --> 0){
      if(*(dir_monitor_info_array[idx]->id) == id && *(dir_monitor_info_array[idx]->bus_type) == bt){
        found = true;
        break;
      }
    }
    tlc_assert(found, "DIRInfo: core ID or agent type not found!");
    return dir_monitor_info_array[idx];
  }

  void register_dir_monitor_info(std::shared_ptr<DIRInfo> p){
    dir_monitor_info_array[dir_monitor_info_array_counter++] = p;
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
  *a_opcode                   = *(info->a_opcode);
  *a_param                    = *(info->a_param);
  *a_size                     = *(info->a_size);
  *a_source                   = *(info->a_source);
  *(uint64_t*)a_address       = *(info->a_address);
  *a_mask                     = *(info->a_mask);
  *a_user_alias               = *(info->a_user_alias);
  *a_user_preferCache         = *(info->a_user_preferCache);
  *a_user_needHint            = *(info->a_user_needHint);
  *a_valid                    = *(info->a_valid);
  *(info->a_ready)            = a_ready;

  //Channel B
  *(info->b_param)            = *b_param;
  *(info->b_address)          = *(const uint64_t*)b_address;
  memcpy(info->b_data.get(), b_data, BEATSIZE);
  *(info->b_needdata)         = (info->b_data[0]) & 0x1;
  *(info->b_alias)            = (info->b_data[0]) >> 1;
  *(info->b_valid)            = b_valid;
  *b_ready                    = *(info->b_ready);

  //Channel C
  *c_opcode                   = *(info->c_opcode);
  *c_param                    = *(info->c_param);
  *c_size                     = *(info->c_size);
  *c_source                   = *(info->c_source);
  *(uint64_t*)c_address       = *(info->c_address);
  memcpy(c_data, info->c_data.get(), BEATSIZE);
  *c_echo_blockisdirty        = *(info->c_echo_blockisdirty);
  *c_valid                    = *(info->c_valid);
  *(info->c_ready)            = c_ready;

  //Channel D
  *(info->d_opcode)           = *d_opcode;
  *(info->d_param)            = *d_param;
  *(info->d_size)             = *d_size;
  *(info->d_source)           = *d_source;
  *(info->d_sink)             = *d_sink;
  *(info->d_denied)           = d_denied;
  memcpy(info->d_data.get(), d_data, BEATSIZE);
  *(info->d_corrupt)          = d_corrupt;
  *(info->d_echo_blockisdirty)= d_echo_blockisdirty;
  *(info->d_valid)            = d_valid;
  *d_ready                    = *(info->d_ready);

  //Channel E
  *e_sink                     = *(info->e_sink);
  *e_valid                    = *(info->e_valid);
  *(info->e_ready)            = e_ready;
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
    *(info->a_ready)      = a_ready;
    *a_valid              = *(info->a_valid);
    *a_opcode             = *(info->a_opcode);
    *a_param              = *(info->a_param);
    *a_size               = *(info->a_size);
    *a_source             = *(info->a_source);
    *(paddr_t*)a_address  = *(info->a_address);
    *a_user_preferCache   = *(info->a_user_preferCache);
    *a_mask               = *(info->a_mask);
    memcpy(a_data, info->a_data.get(), BEATSIZE);

    *d_ready                = *(info->d_ready);
    *(info->d_valid)        = d_valid;
    *(info->d_opcode)       = *d_opcode;
    *(info->d_size)         = *d_size;
    *(info->d_source)       = *d_source;
    *(info->d_denied)       = d_denied;
    *(info->d_corrupt)      = d_corrupt;
    memcpy(info->d_data.get(), d_data, BEATSIZE);
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
    *(info->a_opcode) = *a_opcode;
    *(info->a_param) = *a_param;
    *(info->a_size) = *a_size;
    *(info->a_source) = *a_source;
    *(info->a_address) = *(paddr_t *)a_address;
    (*(info->a_user)).value = *a_user;
    (*(info->a_echo)).value = *a_echo;
    *(info->a_mask) = *a_mask;
    memcpy(info->a_data.get(), a_data, BEATSIZE);
    *(info->a_corrupt) = a_corrupt;
    *(info->a_valid) = a_valid;
    *(info->a_ready) = a_ready;

    //Channel B
    *(info->b_opcode) = *b_opcode;
    *(info->b_param) = *b_param;
    *(info->b_size) = *b_size;
    *(info->b_source) = *b_source;
    *(info->b_address) = *(paddr_t *)b_address;
    *(info->b_mask) = *b_mask;
    memcpy(info->b_data.get(), b_data, BEATSIZE);
    *(info->b_corrupt) = b_corrupt;
    *(info->b_valid) = b_valid;
    *(info->b_ready) = b_ready;

    //Channel C
    *(info->c_opcode) = *c_opcode;
    *(info->c_param) = *c_param;
    *(info->c_size) = *c_size;
    *(info->c_source) = *c_source;
    *(info->c_address) = *(paddr_t *)c_address;
    (*(info->c_user)).value = *c_user;
    (*(info->c_echo)).value = *c_echo;
    memcpy(info->c_data.get(), c_data, BEATSIZE);
    *(info->c_corrupt) = c_corrupt;
    *(info->c_valid) = c_valid;
    *(info->c_ready) = c_ready;

    //Channel D
    *(info->d_opcode) = *d_opcode;
    *(info->d_param) = *d_param;
    *(info->d_size) = *d_size;
    *(info->d_source) = *d_source;
    *(info->d_sink) = *d_sink;
    *(info->d_denied) = d_denied;
    (*(info->d_user)).value = *d_user;
    (*(info->d_echo)).value = *d_echo;
    memcpy(info->d_data.get(), d_data, BEATSIZE);
    *(info->d_corrupt) = d_corrupt;
    *(info->d_valid) = d_valid;
    *(info->d_ready) = d_ready;

    //Channel E
    *(info->e_sink) = *e_sink;
    *(info->e_valid) = e_valid;
    *(info->e_ready) = e_ready;
  }

  void dir_monitor_eval(
  const svBitVecVal*    id,
  const svBitVecVal*    bus_type,

  svBit                 dirWReq_ready,//ready
  svBit                 dirWReq_valid,//valid
  const svBitVecVal*    dirWReq_bits_set,//addr
  const svBitVecVal*    dirWReq_bits_way,//addr
  svBit                 dirWReq_bits_data_dirty,
  const svBitVecVal*    dirWReq_bits_data_state,
  const svBitVecVal*    dirWReq_bits_data_clientStates_0,
  const svBitVecVal*    dirWReq_bits_data_clientStates_1,
  svBit                 dirWReq_bits_data_prefetch,

  svBit                 tagWReq_ready,//ready
  svBit                 tagWReq_valid,//valid
  const svBitVecVal*    tagWReq_bits_set,//addr
  const svBitVecVal*    tagWReq_bits_way,//addr
  const svBitVecVal*    tagWReq_bits_tag,//addr
  
  svBit                 clientDirWReq_ready,//ready
  svBit                 clientDirWReq_valid,//valid
  const svBitVecVal*    clientDirWReq_bits_set,//addr
  const svBitVecVal*    clientDirWReq_bits_way,//addr
  const svBitVecVal*    clientDirWReq_bits_data_0_state,
  const svBitVecVal*    clientDirWReq_bits_data_0_alias,
  const svBitVecVal*    clientDirWReq_bits_data_1_state,
  const svBitVecVal*    clientDirWReq_bits_data_1_alias,
  svBit                 clientTagWreq_ready,//ready
  svBit                 clientTagWreq_valid,//valid
  const svBitVecVal*    clientTagWreq_bits_set,//addr
  const svBitVecVal*    clientTagWreq_bits_way,//addr
  const svBitVecVal*    clientTagWreq_bits_tag//addr
  ){
    uint64_t cid = *(const uint64_t*)id;
    uint8_t  bt  = *(const uint8_t*)bus_type;
    if(tl_interface::dir_monitor_info_array_counter == 0)return;
    std::shared_ptr<tl_interface::DIRInfo> dir_info = tl_interface::find_dir_monitor_info(cid, bt);
    
    *(dir_info->dirWReq_ready) = dirWReq_ready;//ready
    *(dir_info->dirWReq_valid) = dirWReq_valid;//valid
    *(dir_info->dirWReq_bits_set) = *(paddr_t *)dirWReq_bits_set;//addr
    *(dir_info->dirWReq_bits_way) = *dirWReq_bits_way;//addr
    *(dir_info->dirWReq_bits_data_dirty) = dirWReq_bits_data_dirty;
    *(dir_info->dirWReq_bits_data_state) = *dirWReq_bits_data_state;
    *(dir_info->dirWReq_bits_data_clientStates_0) = *dirWReq_bits_data_clientStates_0;
    *(dir_info->dirWReq_bits_data_clientStates_1) = *dirWReq_bits_data_clientStates_1;
    *(dir_info->dirWReq_bits_data_prefetch) = dirWReq_bits_data_prefetch;

    *(dir_info->tagWReq_ready) = tagWReq_ready;//ready
    *(dir_info->tagWReq_valid) = tagWReq_valid;//valid
    *(dir_info->tagWReq_bits_set) = *(paddr_t *)tagWReq_bits_set;//addr
    *(dir_info->tagWReq_bits_way) = *tagWReq_bits_way;//addr
    *(dir_info->tagWReq_bits_tag) = *(paddr_t *)tagWReq_bits_tag;//addr
    
    *(dir_info->clientDirWReq_ready) = clientDirWReq_ready;//ready
    *(dir_info->clientDirWReq_valid) = clientDirWReq_valid;//valid
    *(dir_info->clientDirWReq_bits_set) = *(paddr_t *)clientDirWReq_bits_set;//addr
    *(dir_info->clientDirWReq_bits_way) = *clientDirWReq_bits_way;//addr
    *(dir_info->clientDirWReq_bits_data_0_state) = *clientDirWReq_bits_data_0_state;
    *(dir_info->clientDirWReq_bits_data_0_alias) = *clientDirWReq_bits_data_0_alias;
    *(dir_info->clientDirWReq_bits_data_1_state) = *clientDirWReq_bits_data_1_state;
    *(dir_info->clientDirWReq_bits_data_1_alias) = *clientDirWReq_bits_data_1_alias;
    *(dir_info->clientTagWreq_ready) = clientTagWreq_ready;//ready
    *(dir_info->clientTagWreq_valid) = clientTagWreq_valid;//valid
    *(dir_info->clientTagWreq_bits_set) = *(paddr_t *)clientTagWreq_bits_set;//addr
    *(dir_info->clientTagWreq_bits_way) = *clientTagWreq_bits_way;//addr
    *(dir_info->clientTagWreq_bits_tag) = *(paddr_t *)clientTagWreq_bits_tag;//addr


    
  }
#ifdef __cplusplus
}
#endif