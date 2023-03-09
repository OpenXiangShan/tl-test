#ifndef TLC_TEST_MONITOR_H
#define TLC_TEST_MONITOR_H
#include "../Interface/Interface.h"
#include "../Utils/Common.h"

namespace tl_monitor{

//core 0 d$: [100] [A] [AcquireData NtoT] addr: 400 source: 0 sink: 0 data: 0
//alias: 0 preferCache: 0 needdata: 0 needHint: 0 blockisdirty: 0
using namespace std;
class info_BaseEntry{
  private:
    string sum;
  public:
    string module;
    string cycle;
    string add(){return sum = module + "[" +cycle + "] ";}
};

class info_ChnlEntry{
  private:
    string sum;
  public:
    string channel;
    string op;
    string param;
    string addr;
    string source;
    string sink;
    string data;
    //uesr
    string alias;
    string prefercache;
    string needHint;
    string needdata;
    string blockisdirty;
    //UL-channel
    string ula_add(){return sum = "["+channel +"] " + "[" + op + " " + param + "] " 
                + "addr: " + addr + " source: " + source
                + " preferCache: " + prefercache//user
                + " data: " + data;}//data
    string uld_add(){return sum = "["+channel +"] " + "[" + op + " " + param + "] " 
                + "addr: " + addr + " source: " + source + " sink: " + sink
                + " data: " + data;}//data
    
    //C-channel
    string ca_add(){return sum = "["+channel +"] " + "[" + op + " " + param + "] " 
                + "addr: " + addr + " source: " + source
                + " alias: " + alias + " preferCache: " + prefercache + " needHint: " + needHint//user
                + " data: " + data;}//data

    string cb_add(){return sum = "["+channel +"] " + "[" + op + " " + param + "] " 
                + "addr: " + addr + " source: " + source
                + " alias: " + alias + " needdata: " + needdata//user
                + " data: " + data;}//data
    string cc_add(){return sum = "["+channel +"] " + "[" + op + " " + param + "] " 
                + "addr: " + addr + " source: " + source
                + " blockisdirty: " + blockisdirty//user
                + " data: " + data;}//data
    string cd_add(){return sum = "["+channel +"] " + "[" + op + " " + param + "] " 
                + "addr: " + addr + " source: " + source + " sink: " + sink
                + " blockisdirty: " + blockisdirty//user
                + " data: " + data;}//data
     string ce_add(){return sum = "["+channel +"] " + "[" + op + " " + param + "] " 
                + " sink: " + sink;};
    
    
    void clear_data(){data = "";};
};


using namespace tl_interface;
class Monitor{
  private:
  uint64_t* cycle;
  uint64_t id;
  uint8_t bus_type;
  std::shared_ptr<TLInfo> info;
  info_BaseEntry info_base;
  info_ChnlEntry info_chnl;
  public:
  Monitor(uint64_t* c, uint64_t iid, uint8_t bt);
  std::shared_ptr<TLInfo> get_info();
  void print_info();
  void fire_ULA();
  void fire_ULD();
  void fire_CA();
  void fire_CB();
  void fire_CC();
  void fire_CD();
  void fire_CE();

  void fire_CAgent();
  void fire_ULAgent();
};

class DIR_Monitor{
  private:
  uint64_t* cycle;
  uint64_t id;
  uint8_t bus_type;
  std::shared_ptr<DIRInfo> info;
  public:
  DIR_Monitor(uint64_t* c, uint64_t iid, uint8_t bt);
  std::shared_ptr<DIRInfo> get_info();
  void print_info();
};

}
#endif //TLC_TEST_MONITOR_H