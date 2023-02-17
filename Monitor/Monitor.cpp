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

  void Monitor::print_info(){
    std::string type_string;
    switch(this->bus_type){
      case TILE_BUS_TYPE: type_string = "TILE" + std::to_string(this->id);break;
      case L3_BUS_TYPE: type_string = "L3" + std::to_string(this->id);break;
      case DMA_BUS_TYPE: type_string = "DMA";break;
      default: type_string = "Unkown";
    }

    std::string print_info = type_string + " Monitor is speaking!";
    printf("%s\n", print_info.c_str());
  }

}