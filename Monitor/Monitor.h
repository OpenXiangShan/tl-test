#ifndef TLC_TEST_MONITOR_H
#define TLC_TEST_MONITOR_H
#include "../Interface/Interface.h"
#include "../Utils/Common.h"

namespace tl_monitor{
using namespace tl_interface;
class Monitor{
  private:
  uint64_t* cycle;
  uint64_t id;
  uint8_t bus_type;
  std::shared_ptr<TLInfo> info;
  public:
  Monitor(uint64_t* c, uint64_t iid, uint8_t bt);
  std::shared_ptr<TLInfo> get_info();
  void print_info();
};
}
#endif //TLC_TEST_MONITOR_H