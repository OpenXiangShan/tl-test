//
// Created by ljw on 10/21/21.
//

#ifndef TLC_TEST_BASEAGENT_H
#define TLC_TEST_BASEAGENT_H

#include "../Utils/Common.h"
#include "../Utils/ScoreBoard.h"
#include "Port.h"
#include <set>

namespace tl_agent {

enum Resp { OK, FAIL };

enum {
  S_INVALID = 0,
  S_VALID,
  S_SENDING_A,        // ready to send A request actively
  S_REACTING_B,       // ready to react B request actively
  S_SENDING_C,        // ready to send C request actively
  S_C_WAITING_D,      // C wait for D response
  S_A_WAITING_D,      // A wait for D response
  S_C_WAITING_D_INTR, // C wait for D response while probe interrupted
  S_A_WAITING_D_INTR, // A wait for D response while probe interrupted
  S_SENDING_E,        // ready to send E request actively
};

class ReqField {
public:
  uint32_t value;
};

class RespField {
public:
  uint32_t value;
};

class EchoField {
public:
  uint32_t value;
};

template <typename T> class PendingTrans {
public:
  int beat_cnt;
  int nr_beat;

  std::shared_ptr<T> info;
  PendingTrans() {
    nr_beat = 0;
    beat_cnt = 0;
  }
  ~PendingTrans() = default;

  bool is_multiBeat() { return (this->nr_beat != 1); };
  bool is_pending() { return (beat_cnt != 0); }
  void init(std::shared_ptr<T> info, int nr_beat) {
    this->info = info;
    this->nr_beat = nr_beat;
    beat_cnt = nr_beat;
  }
  void update() {
    beat_cnt--;
    tlc_assert(beat_cnt >= 0, "More beats received than expected!");
  }
};

class IDPool {
private:
  std::set<uint32_t> *idle_ids;
  std::set<uint32_t> *used_ids;
  uint32_t pending_freeid;
  uint32_t init[2];

public:
  IDPool(uint32_t start, uint32_t end) {
    idle_ids = new std::set<uint32_t>();
    used_ids = new std::set<uint32_t>();
    init[0] = start; init[1] = end;
    for (uint32_t i = start; i < end; i++) {
      idle_ids->insert(i);
    }
    used_ids->clear();
    pending_freeid = -1;
  }
  ~IDPool() {
    delete idle_ids;
    delete used_ids;
  }
  void clear(){
    idle_ids->clear();
    used_ids->clear();
    for (uint32_t i = init[0]; i < init[1]; i++) {
      idle_ids->insert(i);
    }
    pending_freeid = -1;
  }
  uint32_t getid() {
    if (idle_ids->size() == 0)
      return -1;
    uint32_t ret = *idle_ids->begin();
    used_ids->insert(ret);
    idle_ids->erase(ret);
    return ret;
  }
  void freeid(uint32_t id) { this->pending_freeid = id; }
  void update() {
    if (pending_freeid != -1) {
      tlc_assert(used_ids->count(pending_freeid) > 0,
                 "Try to free unused SourceID!");
      used_ids->erase(pending_freeid);
      idle_ids->insert(pending_freeid);
      // printf("free %d\n", pending_freeid);
      pending_freeid = -1;
    }
  }
  bool full() { return idle_ids->empty(); }
};

class BaseAgent {
protected:
  std::shared_ptr<Port<ReqField, RespField, EchoField, BEATSIZE>> port;
  GlobalBoard<paddr_t> *globalBoard;
  IDPool a_idpool;
  virtual void timeout_check() = 0;
  int id;
  
public:
  uint64_t core_id;
  uint8_t bus_type;
  virtual std::string type_to_string() = 0;
  virtual Resp send_a(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE> >a) = 0;
  virtual void handle_b(std::shared_ptr<ChnB>b) = 0;
  virtual Resp send_c(std::shared_ptr<ChnC<ReqField, EchoField, DATASIZE> >c) = 0;
  virtual void fire_a() = 0;
  virtual void fire_b() = 0;
  virtual void fire_c() = 0;
  virtual void fire_d() = 0;
  virtual void fire_e() = 0;
  virtual void handle_channel() = 0;
  virtual void update_signal() = 0;
  virtual bool local_probe(paddr_t address) = 0;
  BaseAgent(int a_begin, int a_end) : a_idpool(a_begin, a_end){};
  virtual ~BaseAgent() = default;
  virtual void clear(void) = 0;
};

} // namespace tl_agent

#endif // TLC_TEST_BASEAGENT_H
