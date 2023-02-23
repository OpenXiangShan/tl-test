//
// Created by wkf on 2021/11/2.
//

#ifndef TLC_TEST_CAGENT_H
#define TLC_TEST_CAGENT_H

#include "../Utils/Common.h"
#include "../Utils/ScoreBoard.h"
#include "BaseAgent.h"
#include "../Interface/Interface.h"

namespace tl_agent {

class C_SBEntry {
public:
  uint64_t time_stamp;
  int status[4];
  int privilege[4];
  int pending_privilege[4];
  int dirty[4];

  C_SBEntry(const int status[], const int privilege[], uint64_t &time) {
    this->time_stamp = time;
    for (int i = 0; i < 4; i++) {
      this->privilege[i] = privilege[i];
      this->status[i] = status[i];
    }
  }
  void update_status(int status, uint64_t &time, int alias) {
    this->status[alias] = status;
    this->time_stamp = time;
  }
  void update_priviledge(int priv, uint64_t &time, int alias) {
    this->privilege[alias] = priv;
    this->time_stamp = time;
  }
  void update_pending_priviledge(int priv, uint64_t &time, int alias) {
    this->pending_privilege[alias] = priv;
    this->time_stamp = time;
  }
  void unpending_priviledge(uint64_t &time, int alias) {
    this->privilege[alias] = this->pending_privilege[alias];
    this->pending_privilege[alias] = -1;
    this->time_stamp = time;
  }
  void update_dirty(int dirty, int alias) { this->dirty[alias] = dirty; }
};

class C_IDEntry {
public:
  paddr_t address;
  int alias;
  C_IDEntry(paddr_t &addr, uint8_t &alias) {
    this->address = addr;
    this->alias = alias;
  }
};

class CAgent : public BaseAgent {
private:
  uint64_t *cycles;
  std::shared_ptr<tl_interface::TLInfo> tl_info;
  PendingTrans<ChnA<ReqField, EchoField, DATASIZE>> pendingA;
  PendingTrans<ChnB> pendingB;
  PendingTrans<ChnC<ReqField, EchoField, DATASIZE>> pendingC;
  PendingTrans<ChnD<RespField, EchoField, DATASIZE>> pendingD;
  PendingTrans<ChnE> pendingE;
  /* Here we need a scoreboard called localBoard maintaining address->info
   * For convenience, an idMap(id->addr) is also maintained
   */
  std::shared_ptr<ScoreBoard<paddr_t, C_SBEntry> > localBoard;
  IDPool c_idpool;
  std::shared_ptr<ScoreBoard<int, C_IDEntry> > aidMap;
  std::shared_ptr<ScoreBoard<int, C_IDEntry> > release_idMap;
  std::shared_ptr<ScoreBoard<int, C_IDEntry> > probe_ack_idMap;
  void timeout_check();

public:
  CAgent(GlobalBoard<paddr_t> *const gb, int id, uint64_t *cycles, uint64_t cid,
         uint8_t bt);
  ~CAgent() = default;
  Resp send_a(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE> >a);
  void handle_b(std::shared_ptr<ChnB>b);
  Resp send_c(std::shared_ptr<ChnC<ReqField, EchoField, DATASIZE> >c);
  Resp send_e(std::shared_ptr<ChnE>e);
  void fire_a();
  void fire_b();
  void fire_c();
  void fire_d();
  void fire_e();
  void handle_channel();
  void update_signal();
  bool local_probe(paddr_t address);

  bool do_acquireBlock(paddr_t address, int param, int alias);
  bool do_acquirePerm(paddr_t address, int param, int alias);
  bool do_releaseData(paddr_t address, int param, std::shared_ptr<uint8_t[]>data, int alias);
  bool do_releaseDataAuto(paddr_t address, int alias);
  std::string type_to_string();
  void clear(){
    this->localBoard->clear();
    this->c_idpool.clear();
    this->aidMap->clear();
    this->release_idMap->clear();
    this->probe_ack_idMap->clear();
  }
};

} // namespace tl_agent

#endif // TLC_TEST_CAGENT_H
