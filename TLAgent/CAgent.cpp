//
// Created by wkf on 2021/11/2.
//

#include "CAgent.h"

namespace tl_agent {

    int genPriv(int param) {
        switch (param) {
            case toT: return TIP;
            case toB: return BRANCH;
            case toN: return INVALID;
            default:
                tlc_assert(false, "Invalid param!");
        }
    }

    CAgent::CAgent(GlobalBoard<paddr_t> *const gb, uint64_t *cycles):
        BaseAgent(), pendingA(), pendingD(), pendingE()
    {
        this->globalBoard = gb;
        this->cycles = cycles;
        this->localBoard = new ScoreBoard<paddr_t , C_SBEntry>();
        this->idMap = new ScoreBoard<int, C_IDEntry>();
    }

    Resp CAgent::send_a(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> &a) {
        switch (*a->opcode) {
            case AcquireBlock: {
                std::shared_ptr<C_IDEntry> idmap_entry(new C_IDEntry(*a->address));
                idMap->update(*a->source, idmap_entry);

                if (localBoard->haskey(*a->address)) {
                    localBoard->query(*a->address)->update_status(S_SENDING_A, *cycles);
                } else {
                    std::shared_ptr<C_SBEntry> entry(new C_SBEntry(S_SENDING_A, INVALID, *cycles)); // Set pending as INVALID
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
        *this->port->a.mask = *a->mask;
        *this->port->a.source = *a->source;
        *this->port->a.valid = true;
        return OK;
    }

    void CAgent::handle_b() {

    }

    Resp CAgent::send_c(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> &c) {

    }

    void CAgent::handle_d() {

    }

    Resp CAgent::send_e(std::shared_ptr<ChnE> &e) {
        *this->port->e.sink = *e->sink;
        *this->port->e.valid = true;
        return OK;
    }

    void CAgent::fire_a() {
        if (this->port->a.fire()) {
            auto chnA = this->port->a;
            *chnA.valid = false;
            tlc_assert(pendingA.is_pending(), "No pending A but A fired!");
            pendingA.update();
            if (!pendingA.is_pending()) { // req A finished
                this->localBoard->query(*pendingA.info->address)->update_status(S_WAITING_D, *cycles);
            }
        }
    }

    void CAgent::fire_b() {

    }

    void CAgent::fire_c() {

    }

    void CAgent::fire_d() {
        if (this->port->d.fire()) {
            auto chnD = this->port->d;
            bool hasData = *chnD.opcode == GrantData;
            auto addr = idMap->query(*chnD.source)->address;
            auto info = localBoard->query(addr);
            tlc_assert(info->status == S_WAITING_D, "Status error!");
            if (pendingD.is_pending()) { // following beats
                tlc_assert(*chnD.opcode == *pendingD.info->opcode, "Opcode mismatch among beats!");
                tlc_assert(*chnD.param == *pendingD.info->param, "Param mismatch among beats!");
                tlc_assert(*chnD.source == *pendingD.info->source, "Source mismatch among beats!");
                pendingD.update();
            } else { // new D resp
                std::shared_ptr<ChnD<RespField, EchoField, DATASIZE>> resp_d(new ChnD<RespField, EchoField, DATASIZE>());
                resp_d->opcode = new uint8_t(*chnD.opcode);
                resp_d->param = new uint8_t(*chnD.param);
                resp_d->source = new uint8_t(*chnD.source);
                resp_d->data = hasData ? new uint8_t[DATASIZE] : nullptr;
                int nr_beat = (*chnD.opcode == Grant || *chnD.opcode == ReleaseAck) ? 0 : 1; // TODO: parameterize it
                pendingD.init(resp_d, nr_beat);
            }
            if (hasData) {
                int beat_num = pendingD.nr_beat - pendingD.beat_cnt;
                for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
                    pendingD.info->data[i] = chnD.data[i - BEATSIZE * beat_num];
                }
            }
            if (!pendingD.is_pending()) {
                // ULAgent needn't care about endurance
                if (hasData) {
                    Log("[GrantData] addr: %hx data: ", addr);
                    for(int i = 0; i < DATASIZE; i++) {
                        Log("%02hhx", pendingD.info->data[i]);
                    }
                    Log("\n");
                    this->globalBoard->verify(addr, pendingD.info->data);
                }
                if (*chnD.opcode == GrantData || *chnD.opcode == Grant) {
                    std::shared_ptr<ChnE> req_e(new ChnE());
                    req_e->sink = new uint8_t(*chnD.sink);
                    pendingE.init(req_e, 1);
                    info->update_status(S_SENDING_E, *cycles);
                    info->update_priviledge(genPriv(*chnD.param), *cycles);
                }
                idMap->erase(*chnD.source);
                this->idpool.freeid(*chnD.source);
            }
        }
    }

    void CAgent::fire_e() {
        if (this->port->e.fire()) {
            auto chnE = this->port->e;
            *chnE.valid = false;
            tlc_assert(pendingE.is_pending(), "No pending A but A fired!");
            pendingE.update();
        }
    }

    void CAgent::handle_channel() {
        fire_a();
        fire_d();
        fire_e();
    }

    void CAgent::update_signal() {
        *this->port->d.ready = true; // TODO: do random here
        if (pendingA.is_pending()) {
            // TODO: do delay here
            send_a(pendingA.info);
        } else {
            *this->port->a.valid = false;
        }
        if (pendingE.is_pending()) {
            send_e(pendingE.info);
        }
        // do timeout check lazily
        if (*this->cycles % TIMEOUT_INTERVAL == 0) {
            this->timeout_check();
        }
        idpool.update();
    }

    bool CAgent::do_acquireBlock(paddr_t address, int param) {
        if (pendingA.is_pending() || idpool.full())
            return false;
        if (localBoard->haskey(address)) { // check whether this transaction is legal
            auto entry = localBoard->query(address);
            auto privilege = entry->privilege;
            auto status = entry->status;
            if (status != S_VALID && status != S_INVALID) {
                return false;
            }
            if (privilege == TIP) return false;
            if (privilege == BRANCH && param != BtoT) return false;
            if (privilege == INVALID && param == BtoT) return false;
        }
        std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> req_a(new ChnA<ReqField, EchoField, DATASIZE>());
        req_a->opcode = new uint8_t(AcquireBlock);
        req_a->address = new paddr_t(address);
        req_a->param = new uint8_t(param);
        req_a->size = new uint8_t(ceil(log2((double)DATASIZE)));
        req_a->mask = new uint32_t(0xffffffffUL);
        req_a->source = new uint8_t(this->idpool.getid());
        pendingA.init(req_a, 1);
        Log("[AcquireData] addr: %x\n", address);
        return true;
    }

    void CAgent::timeout_check() {

    }
}