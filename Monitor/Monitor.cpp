#include "Monitor.h"
namespace tl_monitor{
  Monitor::Monitor(uint64_t* c, uint64_t iid, uint8_t bt):
    cycle(c),id(iid), bus_type(bt){
    this->info.reset(new tl_interface::TLInfo(iid, bt));
    tl_interface::register_monitor_info(this->info);
  }

  std::shared_ptr<TLInfo> Monitor::get_info(){
    return this->info;
  }

  using namespace Tool;

  void Monitor::fire_ULA(){
    TLInfo a = *this->info;
    if (*a.a_valid&&*a.a_ready)
    {
      info_chnl.channel = "A";
      switch(*a.a_opcode){
        case PutFullData:
          info_chnl.op = "PutFullData";
          info_chnl.param = "-";
          break;
        case PutPartialData:
          info_chnl.op = "PutPartialData ";
          info_chnl.param = "-";
          break;
        case Get:
          info_chnl.op = "Get";
          info_chnl.param = "-";
          break;
        default:info_chnl.op = "error";
      }

      info_chnl.addr = hex_to_str(*a.a_address,8,false);
      info_chnl.source = std::to_string(*a.a_source);
      //data
      info_chnl.clear_data();
      if(*a.a_opcode == PutFullData || *a.a_opcode == PutPartialData)
        for (int i = BEATSIZE-1; i >= 0; i--)
        {
          info_chnl.data = info_chnl.data + hex_to_str(a.a_data[i],2,false);
        }
      else
        info_chnl.data = "-";
      //user
      info_chnl.alias = std::to_string(*a.a_user_alias);
      info_chnl.prefercache = std::to_string(*a.a_user_preferCache);
      info_chnl.needHint = std::to_string(*a.a_user_needHint);
      //print
      std::string print_info = info_base.add() + info_chnl.ula_add();
      printf("%s\n", print_info.c_str());
    }
  }


  void Monitor::fire_ULD(){
    TLInfo d = *this->info;
    if (*d.d_valid&&*d.d_ready)
    {
      info_chnl.channel = "D";
      switch(*d.d_opcode){
        case AccessAck:
          info_chnl.op = "AccessAck";
          info_chnl.param = "-";
          break;
        case AccessAckData:
          info_chnl.op = "AccessAckData";
          info_chnl.param = "-";
        default:info_chnl.op = "error";
      }

      info_chnl.addr = "-";
      info_chnl.source = std::to_string(*d.d_source);
      info_chnl.sink = std::to_string(*d.d_sink);
      //data
      info_chnl.clear_data();
      if(*d.d_opcode == AccessAckData)
        for (int i = BEATSIZE-1; i >= 0; i--)
        {
          info_chnl.data = info_chnl.data + hex_to_str(d.d_data[i],2,false);
        }
      else
        info_chnl.data = "-";
      //print
      std::string print_info = info_base.add() + info_chnl.uld_add();
      printf("%s\n", print_info.c_str());
    }
  }

  void Monitor::fire_ULAgent(){
    fire_ULA();
    fire_ULD();
  }


  void Monitor::fire_CA(){
    TLInfo a = *this->info;
    if (*a.a_valid&&*a.a_ready)
    {
      info_chnl.channel = "A";
      switch(*a.a_opcode){
        //TL-C
        case AcquireBlock:
          info_chnl.op = "AcquireBlock";
          switch (*a.a_param)
          {
          case NtoT:info_chnl.param = "NtoT";break;
          case NtoB:info_chnl.param = "NtoB";break;
          case BtoT:info_chnl.param = "BtoT";break;
          default:info_chnl.param = "error";break;
          }
          break;
        case AcquirePerm:
          info_chnl.op = "AcquirePerm";
          if(*a.a_param == NtoT)
            info_chnl.param = "NtoT";
          else
            info_chnl.param = "error";
          break;
        //TL-UH
        case Hint:
          info_chnl.op = "Hint";
          info_chnl.param = "-";
        //TL-UL
        case PutFullData:
          info_chnl.op = "PutFullData";
          info_chnl.param = "-";
          break;
        case PutPartialData:
          info_chnl.op = "PutPartialData";
          info_chnl.param = "-";
          break;
        case Get:
          info_chnl.op = "Get";
          info_chnl.param = "-";
          break;
        default:info_chnl.op = "error";break;
      }

      info_chnl.addr = hex_to_str(*a.a_address,8,false);
      info_chnl.source = std::to_string(*a.a_source);
      //data
      info_chnl.clear_data();
      if(*a.a_opcode == PutFullData || *a.a_opcode == PutPartialData)
        for (int i = BEATSIZE-1; i >= 0; i--)
        {
          info_chnl.data = info_chnl.data + hex_to_str(a.a_data[i],2,false);
        }
      else
        info_chnl.data = "-";
      //user
      info_chnl.alias = std::to_string(*a.a_user_alias);
      info_chnl.prefercache = std::to_string(*a.a_user_preferCache);
      info_chnl.needHint = std::to_string(*a.a_user_needHint);
      //print
      std::string print_info = info_base.add() + info_chnl.ca_add();
      printf("%s\n", print_info.c_str());
    }
  }

  void Monitor::fire_CB(){
    TLInfo b = *this->info;
    if (*b.b_valid&&*b.b_ready)
    {
      info_chnl.channel = "B";
      switch(*b.b_opcode){
        case Probe:
          info_chnl.op = "Probe";
          switch (*b.b_param)
          {
          case toT:info_chnl.param = "toT";break;
          case toB:info_chnl.param = "toB";break;
          case toN:info_chnl.param = "toN";break;
          default:info_chnl.param = "error";break;
          }
          break;
        default:info_chnl.op = "error";
      }
      
      info_chnl.addr = hex_to_str(*b.b_address,8,false);
      info_chnl.source = std::to_string(*b.d_source);
      info_chnl.clear_data();
      info_chnl.data = "-";
      //uesr
      info_chnl.alias = std::to_string(*b.b_alias);
      info_chnl.needdata = std::to_string(*b.b_needdata);
      //print
      std::string print_info = info_base.add() + info_chnl.cb_add();
      printf("%s\n", print_info.c_str());
    }
  }

  void Monitor::fire_CC(){
    TLInfo c = *this->info;
    if (*c.c_valid&&*c.c_ready)
    {
      info_chnl.channel = "C";
      switch(*c.c_opcode){
        case ProbeAck:
          info_chnl.op = "ProbeAck";
          switch (*c.c_param)
          {
          case TtoB:info_chnl.param = "TtoB";break;
          case TtoN:info_chnl.param = "TtoN";break;
          case BtoN:info_chnl.param = "BtoN";break;
          case TtoT:info_chnl.param = "TtoTwarning";break;
          case BtoB:info_chnl.param = "BtoBwarning";break;
          case NtoN:info_chnl.param = "NtoNwarning";break;
          default:info_chnl.param = "error";break;
          }
          break;
        case ProbeAckData:
          info_chnl.op = "ProbeAckData";
          switch (*c.c_param)
          {
          case TtoB:info_chnl.param = "TtoB";break;
          case TtoN:info_chnl.param = "TtoN";break;
          case BtoN:info_chnl.param = "BtoN";break;
          case TtoT:info_chnl.param = "TtoTwarning";break;
          case BtoB:info_chnl.param = "BtoBwarning";break;
          case NtoN:info_chnl.param = "NtoNwarning";break;
          default:info_chnl.param = "error";break;
          }
          break;
        case Release:
          info_chnl.op = "Release";
          switch (*c.c_param)
          {
          case TtoB:info_chnl.param = "TtoB";break;
          case TtoN:info_chnl.param = "TtoN";break;
          case BtoN:info_chnl.param = "BtoN";break;
          default:info_chnl.param = "error";break;
          }
          break;
        case ReleaseData:
          info_chnl.op = "ReleaseData";
          switch (*c.c_param)
          {
          case TtoB:info_chnl.param = "TtoB";break;
          case TtoN:info_chnl.param = "TtoN";break;
          case BtoN:info_chnl.param = "BtoN";break;
          default:info_chnl.param = "error";break;
          }
          break;
        default:info_chnl.op = "error";
      }
      info_chnl.addr = hex_to_str(*c.c_address,8,false);
      info_chnl.source = std::to_string(*c.d_source);
      //data
      info_chnl.clear_data();
      if(*c.c_opcode == ProbeAckData || *c.c_opcode == ReleaseData)
        for (int i = BEATSIZE-1; i >= 0; i--)
        {
          info_chnl.data = info_chnl.data + hex_to_str(c.c_data[i],2,false);
        }
      else
        info_chnl.data = "-";
      //uesr
      info_chnl.blockisdirty = std::to_string(*c.c_echo_blockisdirty);
      //print
      std::string print_info = info_base.add() + info_chnl.cc_add();
      printf("%s\n", print_info.c_str());
    }
  }

  void Monitor::fire_CD(){
    TLInfo d = *this->info;
    if (*d.d_valid&&*d.d_ready)
    {
      info_chnl.channel = "D";
      switch(*d.d_opcode){
        case AccessAck:
          info_chnl.op = "AccessAck";
          info_chnl.param = "-";
          break;
        case AccessAckData:
          info_chnl.op = "AccessAckData";
          info_chnl.param = "-";
          break;
        case Grant:
          info_chnl.op = "Grant";
          switch (*d.d_param)
          {
          case toT:info_chnl.param = "toT";break;
          case toB:info_chnl.param = "toB";break;
          case toN:info_chnl.param = "toN";break;
          default:info_chnl.param = "error";break;
          }
          break;
        case GrantData:
          info_chnl.op = "GrantData";
          switch (*d.d_param)
          {
          case toT:info_chnl.param = "toT";break;
          case toB:info_chnl.param = "toB";break;
          case toN:info_chnl.param = "toN";break;
          default:info_chnl.param = "error";break;
          }
          break;
        default:info_chnl.op = "error";
      }

      info_chnl.addr = "-";
      info_chnl.source = std::to_string(*d.d_source);
      info_chnl.sink = std::to_string(*d.d_sink);
      //data
      info_chnl.clear_data();
      if(*d.d_opcode == AccessAckData || *d.d_opcode == GrantData)
        for (int i = BEATSIZE-1; i >= 0; i--)
        {
          info_chnl.data = info_chnl.data + hex_to_str(d.d_data[i],2,false);
        }
      else
        info_chnl.data = "-";
      //user
      info_chnl.blockisdirty = std::to_string(*d.d_echo_blockisdirty);
      //print
      std::string print_info = info_base.add() + info_chnl.cd_add();
      printf("%s\n", print_info.c_str());
    }
  }

  void Monitor::fire_CE(){
    TLInfo e = *this->info;
    if(*e.e_valid && *e.e_ready){
      info_chnl.channel = "E";
      info_chnl.op = "GrantAck";
      info_chnl.param = "-";
      info_chnl.sink = std::to_string(*e.e_sink);
      //print
      std::string print_info = info_base.add() + info_chnl.ce_add();
      printf("%s\n", print_info.c_str());
    }
  }

  void Monitor::fire_CAgent(){
    fire_CA();
    fire_CB();
    fire_CC();
    fire_CD();
    fire_CE();
  }


  //core 0 d$: [100] [A] [AcquireData] [NtoT] addr: 400 source: 0 sink: 0 data: 0
  //alias: 0 preferCache: 0 needdata: 0 needHint: 0 blockisdirty: 0
  void Monitor::print_info(){
    info_base.cycle = std::to_string(*this->cycle);
    switch(this->bus_type){
      case TILE_BUS_TYPE: 
        info_base.module = "core "+ std::to_string(this->id) + " L2: ";
        fire_CAgent();
        break;
      case L3_BUS_TYPE: 
        info_base.module = "L3 - bank" + std::to_string(this->id) + " ";
        fire_CAgent();
        break;
      case DMA_BUS_TYPE: 
        info_base.module = "uncache - DMA: ";
        fire_ULAgent(); 
        break;
      default:printf("Unkown\n");break;
    }
  }

}