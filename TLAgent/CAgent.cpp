//
// Created by wkf on 2021/11/2.
//

#include "CAgent.h"

namespace tl_agent {

int capGenPriv(int param) {
  switch (param) {
  case toT:
    return TIP;
  case toB:
    return BRANCH;
  case toN:
    return INVALID;
  default:
    tlc_assert(false, "Invalid param!");
  }
}

int shrinkGenPriv(int param) {
  switch (param) {
  case TtoT:
    return TIP;
  case BtoB:
  case TtoB:
    return BRANCH;
  case TtoN:
  case BtoN:
  case NtoN:
    return INVALID;
  default:
    tlc_assert(false, "Invalid param!");
  }
}

CAgent::CAgent(GlobalBoard<paddr_t> *const gb, int id, uint64_t *cycles,
               uint64_t cid, uint8_t bt):
  pendingA(), pendingB(), pendingC(), pendingD(), pendingE(),
  BaseAgent(GET_CA_A_ID_BEGIN(bt), GET_CA_A_ID_END(bt)),
  c_idpool(GET_CA_C_ID_BEGIN(bt), GET_CA_C_ID_END(bt)){
  using namespace tl_interface;
  this->core_id = cid;
  this->bus_type = bt;
  this->globalBoard = gb;
  this->id = id;
  this->cycles = cycles;
  this->localBoard.reset(new ScoreBoard<paddr_t, C_SBEntry>());
  this->aidMap.reset(new ScoreBoard<int, C_IDEntry>());
  this->release_idMap.reset(new ScoreBoard<int, C_IDEntry>());
  this->probe_ack_idMap.reset(new ScoreBoard<int, C_IDEntry>());
  this->tl_info.reset(new TLInfo(cid,bt));
  register_tlc_info(this->tl_info);
  this->port.reset(new Port<ReqField, RespField, EchoField, BEATSIZE>());
  this->tl_info->connect(this->port);
}
std::string CAgent::type_to_string(){
  using namespace std;
  string mhartid = "core " + to_string(this->core_id);
  string type;
  if(this->bus_type == DCACHE_BUS_TYPE) type = " d$";
  else type = " i$";
  return mhartid + type;
}

Resp CAgent::send_a(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE> >a) {
  switch (*a->opcode) {
  case AcquireBlock: {
    std::shared_ptr<C_IDEntry> idmap_entry(
        new C_IDEntry(*a->address, *a->alias));
    aidMap->update(*a->source, idmap_entry);

    if (localBoard->haskey(*a->address)) {
      localBoard->query(*a->address)
          ->update_status(S_SENDING_A, *cycles, *a->alias);
    } else {
      int statuses[4] = {S_INVALID};
      int privileges[4] = {INVALID};
      for (int i = 0; i < 4; i++) {
        if (*a->alias == i) {
          statuses[i] = S_SENDING_A;
        }
      }
      std::shared_ptr<C_SBEntry> entry(new C_SBEntry(
          statuses, privileges, *cycles)); // Set pending as INVALID
      localBoard->update(*a->address, entry);
    }
    break;
  }
  case AcquirePerm: {
    std::shared_ptr<C_IDEntry> idmap_entry(
        new C_IDEntry(*a->address, *a->alias));
    aidMap->update(*a->source, idmap_entry);
    if (localBoard->haskey(*a->address)) {
      localBoard->query(*a->address)
          ->update_status(S_SENDING_A, *cycles, *a->alias);
    } else {
      int statuses[4] = {S_INVALID};
      int privileges[4] = {INVALID};
      for (int i = 0; i < 4; i++) {
        if (*a->alias == i) {
          statuses[i] = S_SENDING_A;
        }
      }
      std::shared_ptr<C_SBEntry> entry(new C_SBEntry(
          statuses, privileges, *cycles)); // Set pending as INVALID
      localBoard->update(*a->address, entry);
    }
    break;
  }
  default:
    tlc_assert(false, "Unknown opcode for channel A!");
  }
  *this->port->a.opcode = *a->opcode;
  *this->port->a.address = *a->address;
  *this->port->a.size = *a->size;
  *this->port->a.param = *a->param;
  *this->port->a.mask = *a->mask;
  *this->port->a.source = *a->source;
  *this->port->a.alias = *a->alias;
  *this->port->a.valid = true;
  return OK;
}

void CAgent::handle_b(std::shared_ptr<ChnB>b) {
  if (pendingC.is_pending()) {
    Log("[info] B wanna pendingC\n");
    return;
  }
  if (this->c_idpool.full()) {
    Log("[info] id pool full\n");
    return;
  }

  tlc_assert(localBoard->haskey(*b->address), "Probe an non-exist block!");

  auto info = localBoard->query(*b->address);
  auto exact_status = info->status[*b->alias];
  auto exact_privilege = info->privilege[*b->alias];
  tlc_assert(exact_status != S_SENDING_C,
             "handle_b should be mutual exclusive with pendingC!");
  if (exact_status == S_C_WAITING_D) {
    // Probe waits for releaseAck
    return;
  }
  std::shared_ptr<ChnC<ReqField, EchoField, DATASIZE> >req_c(new ChnC<ReqField, EchoField, DATASIZE>());
  req_c->address.reset(new paddr_t(*b->address));
  req_c->size.reset(new uint8_t(*b->size));
  req_c->source.reset(new uint32_t(this->c_idpool.getid()));
  req_c->dirty.reset(new uint8_t(1));
  req_c->alias.reset(new uint8_t(*b->alias));
  // Log("== id == handleB %d\n", *req_c->source);
  if (exact_status == S_INVALID) {
    Log("Probe an non-exist block, status: %d\n", exact_status);
    req_c->opcode.reset(new uint8_t(ProbeAck));
    req_c->param.reset(new uint8_t(NtoN));
    pendingC.init(req_c, 1);
    Log("[%ld] [C] [ProbeAck NtoN] addr: %lx\n", *cycles, *b->address);
  } else if(exact_status == S_SENDING_A || exact_status == S_A_WAITING_D){
    if(exact_privilege == BRANCH){
      req_c->opcode.reset(new uint8_t(ProbeAck));
      req_c->param.reset(new uint8_t(BtoN));
      pendingC.init(req_c, 1);
      Log("[%ld] [C] [ProbeAck BtoN] addr: %lx\n", *cycles, *b->address);
    } else if(exact_privilege == INVALID){
      Log("Probe an non-exist block, status: %d\n", exact_status);
      req_c->opcode.reset(new uint8_t(ProbeAck));
      req_c->param.reset(new uint8_t(NtoN));
      pendingC.init(req_c, 1);
      Log("[%ld] [C] [ProbeAck NtoN] addr: %lx\n", *cycles, *b->address);
    } else {
      tlc_assert(false, "Illegal block status!\n");
    }
  } else {
    int dirty =
        (exact_privilege == TIP) && (info->dirty[*b->alias] || rand() % 3);
    // When should we probeAck with data? request need_data or dirty itself
    req_c->opcode.reset((dirty || *b->needdata) ? new uint8_t(ProbeAckData): new uint8_t(ProbeAck));
    if (*b->param == toB) {
      switch (exact_privilege) {
      case TIP:
        req_c->param.reset(new uint8_t(TtoB));
        break;
      case BRANCH:
        req_c->param.reset(new uint8_t(BtoB));
        break;
      default:
        tlc_assert(false, "Try to probe toB an invalid block!");
      }
    } else if (*b->param == toN) {
      switch (exact_privilege) {
      case TIP:
        req_c->param.reset(new uint8_t(TtoN));
        break;
      case BRANCH:
        req_c->param.reset(new uint8_t(BtoN));
        break;
      default:
        tlc_assert(false, "Try to probe toB an invalid block!");
      }
    }
    if (!globalBoard->haskey(*b->address)) {
      // want to probe an all-zero block which does not exist in global board
      Log("probeAck Data all-zero\n");
      uint8_t *all_zero = new uint8_t[DATASIZE];
      for (int i = 0; i < DATASIZE; i++) {
        all_zero[i] = 0;
      }
      req_c->data.reset(all_zero);
    } else {
      if (*req_c->opcode == ProbeAckData && (*req_c->param == TtoN || *req_c->param == TtoB)) {
        uint8_t *random = new uint8_t[DATASIZE];
        for (int i = 0; i < DATASIZE; i++) {
          random[i] = (uint8_t)rand();
        }
        req_c->data.reset(random);
      } else {
        req_c->data = globalBoard->query(*b->address)->data;
      }
    }
    if (*req_c->opcode == ProbeAckData) {
      pendingC.init(req_c, DATASIZE / BEATSIZE);
    } else {
      pendingC.init(req_c, 1);
    }

    if (*req_c->param == TtoN) {
      Log("[%ld] [C] [ProbeAck TtoN] addr: %lx data: ", *cycles, *b->address);
    } else if (*req_c->param == TtoB) {
      Log("[%ld] [C] [ProbeAck TtoB] addr: %lx data: ", *cycles, *b->address);
    } else if (*req_c->param == NtoN) {
      Log("[%ld] [C] [ProbeAck NtoN] addr: %lx data: ", *cycles, *b->address);
    } else if (*req_c->param == BtoN) {
      Log("[%ld] [C] [ProbeAck BtoN] addr: %lx data: ", *cycles, *b->address);
    } else if (*req_c->param == BtoB) {
      Log("[%ld] [C] [ProbeAck BtoB] addr: %lx data: ", *cycles, *b->address);
    } else {
      tlc_assert(false, "What the hell is req_c's param?");
    }
    if (*req_c->opcode == ProbeAckData) {
      for (int i = 0; i < DATASIZE; i++) {
        Dump("%02hhx", req_c->data[DATASIZE - 1 - i]);
      }
    } else {
      Dump("no data");
    }
    Dump("\n");
  }
  pendingB.update();
}

Resp CAgent::send_c(std::shared_ptr<ChnC<ReqField, EchoField, DATASIZE> >c) {
  switch (*c->opcode) {
  case ReleaseData: {
    std::shared_ptr<C_IDEntry> idmap_entry(
        new C_IDEntry(*c->address, *c->alias));
    release_idMap->update(*c->source, idmap_entry);

    if (localBoard->haskey(*c->address)) {
      localBoard->query(*c->address)
          ->update_status(S_SENDING_C, *cycles, *c->alias);
    } else {
      tlc_assert(false, "Localboard key not found!");
    }
    int beat_num = pendingC.nr_beat - pendingC.beat_cnt;
    for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
      this->port->c.data[i - BEATSIZE * beat_num] = c->data[i];
    }
    break;
  }
  case ProbeAckData: {
    std::shared_ptr<C_IDEntry> idmap_entry(
        new C_IDEntry(*c->address, *c->alias));
    probe_ack_idMap->update(*c->source, idmap_entry);

    if (localBoard->haskey(*c->address)) {
      // TODO: What if this is an interrupted probe?
      localBoard->query(*c->address)
          ->update_status(S_SENDING_C, *cycles, *c->alias);
    } else {
      tlc_assert(false, "Localboard key not found!");
    }
    int beat_num = pendingC.nr_beat - pendingC.beat_cnt;
    for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
      this->port->c.data[i - BEATSIZE * beat_num] = c->data[i];
    }
    break;
  }
  case ProbeAck: {
    std::shared_ptr<C_IDEntry> idmap_entry(
        new C_IDEntry(*c->address, *c->alias));
    probe_ack_idMap->update(*c->source, idmap_entry);
    // tlc_assert(*c->param == NtoN, "Now probeAck only supports NtoN");
    if (localBoard->haskey(*c->address)) {
      auto item = localBoard->query(*c->address);
      switch(item->status[*c->alias]){
        case S_C_WAITING_D: item->update_status(S_C_WAITING_D_INTR, *cycles, *c->alias);break;
        case S_A_WAITING_D: item->update_status(S_A_WAITING_D_INTR, *cycles, *c->alias);break;
        case S_C_WAITING_D_INTR: break;
        case S_A_WAITING_D_INTR: break;
        default: item->update_status(S_SENDING_C, *cycles, *c->alias);break;
      }
    } else {
      tlc_assert(false, "Localboard key not found!");
    }
    break;
  }
  default:
    tlc_assert(false, "Unknown opcode for channel C!");
  }
  *this->port->c.opcode = *c->opcode;
  *this->port->c.param = *c->param;
  *this->port->c.address = *c->address;
  *this->port->c.size = *c->size;
  *this->port->c.source = *c->source;
  *this->port->c.dirty = *c->dirty;
  *this->port->c.valid = true;
  return OK;
}

Resp CAgent::send_e(std::shared_ptr<ChnE>e) {
  *this->port->e.sink = *e->sink;
  *this->port->e.valid = true;
  return OK;
}

void CAgent::fire_a() {
  if (this->port->a.fire()) {
    auto chnA = this->port->a;
    // Log("[%ld] [A fire] addr: %lx\n", *cycles, *chnA.address);
    *chnA.valid = false;
    tlc_assert(pendingA.is_pending(), "No pending A but A fired!");
    pendingA.update();
    if (!pendingA.is_pending()) { // req A finished
      this->localBoard->query(*pendingA.info->address)
          ->update_status(S_A_WAITING_D, *cycles, *pendingA.info->alias);
    }
  }
}

void CAgent::fire_b() {
  if (this->port->b.fire()) {
    auto chnB = this->port->b;
    // Log("[%ld] [B fire] addr: %lx\n", *cycles, *chnB.address);
    std::shared_ptr<ChnB>req_b(new ChnB());
    req_b->opcode.reset(new uint8_t(*chnB.opcode));
    req_b->address.reset(new paddr_t(*chnB.address));
    req_b->param.reset(new uint8_t(*chnB.param));
    req_b->size.reset(new uint8_t(*chnB.size));
    req_b->source.reset(new uint32_t(*chnB.source));
    req_b->alias.reset(new uint8_t(*chnB.alias));
    req_b->needdata.reset(new uint8_t(*chnB.needdata));
    pendingB.init(req_b, 1);
    std::string param_str = *chnB.param==toN? "toN":(*chnB.param==toB? "toB":(*chnB.param==toT? "toT":"Unknown"));
    Log("[%ld] [B] [Probe %s] addr: %lx alias: %d\n", *cycles, param_str.c_str(), *chnB.address,
        (*chnB.alias) >> 1);
  }
}

void CAgent::fire_c() {
  if (this->port->c.fire()) {
    auto chnC = this->port->c;
    bool releaseHasData = *chnC.opcode == ReleaseData;
    bool needAck = *chnC.opcode == ReleaseData || *chnC.opcode == Release;
    bool probeAckDataToB = *chnC.opcode == ProbeAckData &&
                           (*chnC.param == TtoB || *chnC.param == BtoB);
    tlc_assert(pendingC.is_pending(), "No pending C but C fired!");
    pendingC.update();
    if (!pendingC.is_pending()) { // req C finished
      *chnC.valid = false;
      // Log("[%ld] [C fire] addr: %lx opcode: %lx\n", *cycles, *chnC.address,
      // *chnC.opcode);
      auto info = this->localBoard->query(*pendingC.info->address);
      auto exact_status = info->status[*pendingC.info->alias];
      if (needAck) {
        info->update_status(S_C_WAITING_D, *cycles, *pendingC.info->alias);
      } else {
        if (exact_status == S_C_WAITING_D_INTR) {
          info->update_status(S_C_WAITING_D, *cycles, *pendingC.info->alias);
        } else if (exact_status == S_A_WAITING_D_INTR ||
                   exact_status == S_A_WAITING_D) {
          info->update_status(S_A_WAITING_D, *cycles, *pendingC.info->alias);
        } else {
          if (probeAckDataToB) {
            info->update_status(S_VALID, *cycles, *pendingC.info->alias);
          } else {
            info->update_status(S_INVALID, *cycles, *pendingC.info->alias);
          }
        }
      }
      if (releaseHasData) {
        std::shared_ptr<Global_SBEntry> global_SBEntry(new Global_SBEntry());
        global_SBEntry->pending_data = pendingC.info->data;
        if (this->globalBoard->get().count(*pendingC.info->address) == 0) {
          global_SBEntry->data = nullptr;
        } else {
          global_SBEntry->data =
              this->globalBoard->get()[*pendingC.info->address]->data;
        }
        global_SBEntry->status = Global_SBEntry::SB_PENDING;
        global_SBEntry->mask = FULLMASK;
        this->globalBoard->update(*pendingC.info->address, global_SBEntry);
      }
      if (*chnC.opcode == ProbeAckData || *chnC.opcode == ProbeAck) {
        std::shared_ptr<Global_SBEntry> global_SBEntry(new Global_SBEntry());
        bool origin_entry_is_pending = false;
        global_SBEntry->data = pendingC.info->data;
        if (this->globalBoard->get().count(*pendingC.info->address) != 0) {
          auto originEntry = this->globalBoard->get()[*pendingC.info->address];
          global_SBEntry->pending_data = originEntry->pending_data;
          if(*chnC.opcode == ProbeAck)global_SBEntry->data = originEntry->data; 
          if (originEntry->status == Global_SBEntry::SB_PENDING) {
            global_SBEntry->status = Global_SBEntry::SB_PENDING;
            origin_entry_is_pending = true;
            global_SBEntry->mask =
                this->globalBoard->get()[*pendingC.info->address]->mask;
          } else {
            global_SBEntry->status = Global_SBEntry::SB_VALID;
            global_SBEntry->mask = FULLMASK;
          }
        } else {
          if(*chnC.opcode == ProbeAck){
            global_SBEntry->pending_data.reset(new uint8_t[DATASIZE]);
            memset(global_SBEntry->pending_data.get(), 0, DATASIZE);
            global_SBEntry->data = nullptr;
          }
          global_SBEntry->status = Global_SBEntry::SB_VALID;
          global_SBEntry->mask = FULLMASK;
        }
        this->globalBoard->update(*pendingC.info->address, global_SBEntry);
        if(!origin_entry_is_pending)this->globalBoard->unpending(*pendingC.info->address);
      }
      if (*chnC.opcode == ReleaseData || *chnC.opcode == Release) {
        info->update_pending_priviledge(shrinkGenPriv(*pendingC.info->param),
                                        *cycles, *pendingC.info->alias);
      } else {
        if (*chnC.opcode == ProbeAck || *chnC.opcode == ProbeAckData) {
          info->update_priviledge(shrinkGenPriv(*pendingC.info->param), *cycles,
                                  *pendingC.info->alias);
        }
        // Log("== free == fireC %d\n", *chnC.source);
        this->c_idpool.freeid(*chnC.source);
      }
    }
  }
}

void CAgent::fire_d() {
  if (this->port->d.fire()) {
    auto chnD = this->port->d;
    bool hasData = *chnD.opcode == GrantData;
    bool grant = *chnD.opcode == GrantData || *chnD.opcode == Grant;
    auto addr = grant? (aidMap->query(*chnD.source)->address):(release_idMap->query(*chnD.source)->address);
    auto alias = grant? (aidMap->query(*chnD.source)->alias):(release_idMap->query(*chnD.source)->alias);
    auto info = localBoard->query(addr);
    auto exact_status = info->status[alias];
    if (!(exact_status == S_C_WAITING_D || exact_status == S_A_WAITING_D ||
          exact_status == S_C_WAITING_D_INTR ||
          exact_status == S_A_WAITING_D_INTR || exact_status == S_INVALID)) {
      printf("fire_d: status of localboard is %d\n", exact_status);
      printf("addr: 0x%lx\n", addr);
      tlc_assert(false, "Status error!");
    }
    if (pendingD.is_pending()) { // following beats
      tlc_assert(*chnD.opcode == *pendingD.info->opcode,
                 "Opcode mismatch among beats!");
      tlc_assert(*chnD.param == *pendingD.info->param,
                 "Param mismatch among beats!");
      tlc_assert(*chnD.source == *pendingD.info->source,
                 "Source mismatch among beats!");
      pendingD.update();
    } else { // new D resp
      std::shared_ptr<ChnD<RespField, EchoField, DATASIZE> >resp_d(new ChnD<RespField, EchoField, DATASIZE>());
      resp_d->opcode.reset(new uint8_t(*chnD.opcode));
      resp_d->param.reset(new uint8_t(*chnD.param));
      resp_d->source.reset(new uint32_t(*chnD.source));
      resp_d->data.reset(grant ? new uint8_t[DATASIZE] : nullptr);
      int nr_beat = (*chnD.opcode == Grant || *chnD.opcode == ReleaseAck)
                        ? 0
                        : 1; // TODO: parameterize it
      pendingD.init(resp_d, nr_beat);
    }
    if (hasData) {
      int beat_num = pendingD.nr_beat - pendingD.beat_cnt;
      for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
        pendingD.info->data[i] = chnD.data[i - BEATSIZE * beat_num];
      }
    }
    if (!pendingD.is_pending()) {
      uint8_t p = *(chnD.param);
      std::string param_str = p==toT? "toT":(p==toB? "toB":(p==toN? "toN":"Unknown"));
      switch (*chnD.opcode) {
      case GrantData: {
        Log("[%ld] [D] [GrantData %s] addr: %lx source: %d sink: %d, data: ", *cycles, param_str.c_str(), addr, *(chnD.source), *(chnD.sink));
        for (int i = 0; i < DATASIZE; i++) {
          Dump("%02hhx", pendingD.info->data[DATASIZE - 1 - i]);
        }
        Dump("\n");
        this->globalBoard->verify(addr, pendingD.info->data);
        info->update_dirty(*chnD.dirty, alias);
        this->a_idpool.freeid(*chnD.source);
        aidMap->erase(*chnD.source);
        break;
      }
      case Grant: {
        Log("[%ld] [D] [Grant %s] addr: %lx source: %d sink: %d\n", *cycles, param_str.c_str(), addr, *(chnD.source), *(chnD.sink));
        info->update_dirty(*chnD.dirty, alias);
        this->a_idpool.freeid(*chnD.source);
        aidMap->erase(*chnD.source);
        break;
      }
      case ReleaseAck: {
        Log("[%ld] [D] [ReleaseAck] addr: %lx source: %d \n", *cycles, addr, *(chnD.source));
        if (exact_status == S_C_WAITING_D) {
          info->update_status(S_INVALID, *cycles, alias);
          info->update_dirty(0, alias);
        } else {
          tlc_assert(exact_status == S_C_WAITING_D_INTR, "Status error!");
          info->update_status(S_SENDING_C, *cycles, alias);
        }
        info->unpending_priviledge(*cycles, alias);
        this->globalBoard->unpending(addr);
        this->c_idpool.freeid(*chnD.source);
        release_idMap->erase(*chnD.source);
        break;
      }
      default:
        tlc_assert(false, "Unknown opcode in channel D!");
      }

      // Send E
      if (grant) {
        tlc_assert(exact_status != S_A_WAITING_D_INTR,
                   "TODO: check this Ridiculous probe!");
        std::shared_ptr<ChnE>req_e(new ChnE());
        req_e->sink.reset(new uint32_t(*chnD.sink));
        req_e->addr.reset(new paddr_t(addr));
        req_e->alias.reset(new uint8_t(alias));
        if (pendingE.is_pending()) {
          Log("New E request when E is pending!\n");
          tlc_assert(false, "E is pending!");
        }
        pendingE.init(req_e, 1);
        info->update_status(S_SENDING_E, *cycles, alias);
        info->update_priviledge(capGenPriv(*chnD.param), *cycles, alias);
        Log("[%ld] [E] [GrantAck] addr: %lx sink: %d\n", *cycles, *(req_e->addr), *(req_e->sink));
      }
      // Log("== free == fireD %d\n", *chnD.source);
    }
  }
}

void CAgent::fire_e() {
  if (this->port->e.fire()) {
    auto chnE = this->port->e;
    *chnE.valid = false;
    tlc_assert(pendingE.is_pending(), "No pending A but E fired!");
    auto info = localBoard->query(*pendingE.info->addr);
    info->update_status(S_VALID, *cycles, *pendingE.info->alias);
    pendingE.update();
  }
}

void CAgent::handle_channel() {
  // Constraint: fire_e > fire_d, otherwise concurrent D/E requests will disturb
  // the pendingE
  fire_a();
  fire_b();
  fire_c();
  fire_d();
  fire_e(); 
}

void CAgent::update_signal() {
  *this->port->d.ready = !(pendingE.is_pending());
  *this->port->b.ready = !(pendingB.is_pending());

  if (pendingA.is_pending()) {
    // TODO: do delay here
    send_a(pendingA.info);
  }
  if (pendingB.is_pending()) {
    handle_b(pendingB.info);
  }
  if (pendingC.is_pending()) {
    send_c(pendingC.info);
  }
  if (pendingE.is_pending()) {
    send_e(pendingE.info);
  }
  // do timeout check lazily
  if (*this->cycles % TIMEOUT_INTERVAL == 0) {
    this->timeout_check();
  }
  a_idpool.update();
  c_idpool.update();
}

bool CAgent::do_acquireBlock(paddr_t address, int param, int alias) {
  if(bus_type == ICACHE_BUS_TYPE){
    tlc_assert(param == NtoB, "ICache can only acquire block with NtoB!\n");
  }
  if (pendingA.is_pending() || pendingB.is_pending() || a_idpool.full())
    return false;
  if (localBoard->haskey(address)) { // check whether this transaction is legal
    auto entry = localBoard->query(address);
    auto privilege = entry->privilege[alias];
    auto status = entry->status[alias];
    if (status != S_VALID && status != S_INVALID) {
      return false;
    }
    if (status == S_VALID) {
      if (privilege == TIP)
        return false;
      // if (privilege == BRANCH && param != BtoT) { param = BtoT; }
      if (privilege == BRANCH && param != BtoT)
        return false;
      if (privilege == INVALID && param == BtoT)
        return false;
    }
  }
  std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE> >req_a(new ChnA<ReqField, EchoField, DATASIZE>());
  req_a->opcode.reset(new uint8_t(AcquireBlock));
  req_a->address.reset(new paddr_t(address));
  req_a->param.reset(new uint8_t(param));
  req_a->size.reset(new uint8_t(ceil(log2((double)DATASIZE))));
  req_a->mask.reset(new uint32_t(0xffffffffUL));
  req_a->source.reset(new uint32_t(this->a_idpool.getid()));
  req_a->alias.reset(new uint8_t(alias));
  // Log("== id == acquire %d\n", *req_a->source);
  pendingA.init(req_a, 1);
  switch (param) {
  case NtoB:
    Log("[%ld] [A] [AcquireData NtoB] addr: %lx source: %d alias: %d\n", *cycles, address,
        *(req_a->source), alias);
    break;
  case NtoT:
    Log("[%ld] [A] [AcquireData NtoT] addr: %lx source: %d alias: %d\n", *cycles, address,
        *(req_a->source), alias);
    break;
  }

  return true;
}

bool CAgent::do_acquirePerm(paddr_t address, int param, int alias) {
  if(bus_type == ICACHE_BUS_TYPE){
    tlc_assert(param == NtoB, "ICache can only acquire block with NtoB!\n");
  }
  if (pendingA.is_pending() || pendingB.is_pending() || a_idpool.full())
    return false;
  if (localBoard->haskey(address)) {
    auto entry = localBoard->query(address);
    auto privilege = entry->privilege[alias];
    auto status = entry->status[alias];
    if (status != S_VALID && status != S_INVALID) {
      return false;
    }
    if (status == S_VALID) {
      if (privilege == TIP)
        return false;
      if (privilege == BRANCH && param != BtoT) {
        param = BtoT;
      }
      if (privilege == INVALID && param == BtoT)
        return false;
    }
  }
  std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE> >req_a(new ChnA<ReqField, EchoField, DATASIZE>());
  req_a->opcode.reset(new uint8_t(AcquirePerm));
  req_a->address.reset(new paddr_t(address));
  req_a->param.reset(new uint8_t(param));
  req_a->size.reset(new uint8_t(ceil(log2((double)DATASIZE))));
  req_a->mask.reset(new uint32_t(0xffffffffUL));
  req_a->source.reset(new uint32_t(this->a_idpool.getid()));
  req_a->alias.reset(new uint8_t(alias));
  // Log("== id == acquire %d\n", *req_a->source);
  pendingA.init(req_a, 1);
  std::string param_str = param == NtoB? "NtoB":(param == NtoT? "NtoT":(param == BtoT?"BtoT":"Unknown"));
  Log("[%ld] [A] [AcquirePerm %s] addr: %lx source: %d alias: %d\n", *cycles, param_str.c_str(), address, *(req_a->source), alias);
  return true;
}

bool CAgent::do_releaseData(paddr_t address, int param, std::shared_ptr<uint8_t[]>data,
                            int alias) {
  if (pendingC.is_pending() || pendingB.is_pending() || c_idpool.full() ||
      !localBoard->haskey(address))
    return false;
  // TODO: checkout pendingA
  // TODO: checkout pendingB - give way?
  auto entry = localBoard->query(address);
  auto privilege = entry->privilege[alias];
  auto status = entry->status[alias];
  if (status != S_VALID) {
    return false;
  }
  if (privilege == INVALID)
    return false;
  if (privilege == BRANCH && param != BtoN)
    return false;
  if (privilege == TIP && param == BtoN)
    return false;

  std::shared_ptr<ChnC<ReqField, EchoField, DATASIZE> >req_c(new ChnC<ReqField, EchoField, DATASIZE>());
  req_c->opcode.reset(new uint8_t(ReleaseData));
  req_c->address.reset(new paddr_t(address));
  req_c->param.reset(new uint8_t(param));
  req_c->size.reset(new uint8_t(ceil(log2((double)DATASIZE))));
  req_c->source.reset(new uint32_t(this->c_idpool.getid()));
  req_c->dirty.reset(new uint8_t(1));
  // Log("== id == release %d\n", *req_c->source);
  req_c->data = data;
  req_c->alias.reset(new uint8_t(alias));
  pendingC.init(req_c, DATASIZE / BEATSIZE);
  Log("[%ld] [C] [ReleaseData] addr: %lx, data: ", *cycles, address);
  for (int i = 0; i < DATASIZE; i++) {
    Dump("%02hhx", data[DATASIZE - 1 - i]);
  }
  Dump("\n");
  return true;
}

bool CAgent::do_releaseDataAuto(paddr_t address, int alias) {
  if (pendingC.is_pending() || pendingB.is_pending() || c_idpool.full() ||
      !localBoard->haskey(address))
    return false;
  // TODO: checkout pendingA
  // TODO: checkout pendingB - give way?
  auto entry = localBoard->query(address);
  auto privilege = entry->privilege[alias];
  int param;
  switch (privilege) {
  case INVALID:
    return false;
  case BRANCH:
    param = BtoN;
    break;
  case TIP:
    param = TtoN;
    break;
  default:
    tlc_assert(false, "Invalid priviledge detected!");
  }
  auto status = entry->status[alias];
  if (status != S_VALID) {
    return false;
  }

  std::shared_ptr<ChnC<ReqField, EchoField, DATASIZE> >req_c(new ChnC<ReqField, EchoField, DATASIZE>());
  req_c->opcode.reset(new uint8_t(ReleaseData));
  req_c->address.reset(new paddr_t(address));
  req_c->param.reset(new uint8_t(param));
  req_c->size.reset(new uint8_t(ceil(log2((double)DATASIZE))));
  req_c->source.reset(new uint32_t(this->c_idpool.getid()));
  req_c->dirty.reset(new uint8_t(1));
  req_c->alias.reset(new uint8_t(alias));
  if (param == BtoN) {
    std::shared_ptr<uint8_t[]>data = globalBoard->query(address)->data;
    req_c->data = data;
  } else {
    tlc_assert(param == TtoN, "Wrong execution path!");
    std::shared_ptr<uint8_t[]>putdata(new uint8_t[DATASIZE]);
    for (int i = 0; i < DATASIZE; i++) {
      putdata[i] = (uint8_t)rand();
    }
    req_c->data = putdata;
  }

  // Log("== id == release %d\n", *req_c->source);
  pendingC.init(req_c, DATASIZE / BEATSIZE);
  switch (param) {
  case BtoN:
    Log("[%ld] [C] [ReleaseData BtoN] addr: %lx source: %d data: ", *cycles, address, *(req_c->source));
    break;
  case TtoN:
    Log("[%ld] [C] [ReleaseData TtoN] addr: %lx source: %d data: ", *cycles, address, *(req_c->source));
    break;
  }

  for (int i = 0; i < DATASIZE; i++) {
    Dump("%02hhx", req_c->data[DATASIZE - i - 1]);
  }
  Dump("\n");
  return true;
}

void CAgent::timeout_check() {
  if (localBoard->get().empty()) {
    return;
  }
  for (auto it = this->localBoard->get().begin();
       it != this->localBoard->get().end(); it++) {
    auto value = it->second;
    for (int i = 0; i < 4; i++) {
      if (value->status[i] != S_INVALID && value->status[i] != S_VALID) {
        if (*this->cycles - value->time_stamp > TIMEOUT_INTERVAL) {
          Log("Time Out!\n");
          printf("Now time:   %lu\n", *this->cycles);
          printf("Last stamp: %lu\n", value->time_stamp);
          printf("Status[0]:  %d\n", value->status[0]);
          printf("Address:    0x%lx\n", it->first);
          tlc_assert(false, "Transaction time out");
        }
      }
    }
  }
}

bool CAgent::local_probe(paddr_t address) {
  // can exist in either localboard, pendingA or pendingC
  // TODO: how about unaligned address?
  if (!localBoard->haskey(address))
    return false;
  if (pendingA.is_pending() && *pendingA.info->address == address)
    return true;
  if (pendingC.is_pending() && *pendingC.info->address == address)
    return true;
  std::shared_ptr<tl_agent::C_SBEntry> entry = localBoard->query(address);
  for (int i = 0; i < 4; i++) {
    if (entry->status[i] != S_INVALID && entry->status[i] != S_VALID)
      return true;
  }
  return false;
}
} // namespace tl_agent
