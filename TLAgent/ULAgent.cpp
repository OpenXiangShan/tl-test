//
// Created by ljw on 10/21/21.
//

#include "ULAgent.h"

namespace tl_agent {

    ULAgent::ULAgent(GlobalBoard<paddr_t> *gb, int id, uint64_t* cycles):
            BaseAgent(), pendingA(), pendingD()
    {
        this->globalBoard = gb;
        this->id = id;
        this->cycles = cycles;
        localBoard = new ScoreBoard<int, UL_SBEntry>();
    }

    Resp ULAgent::send_a(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> &a) {
        switch (*a->opcode) {
            case Get: {
                std::shared_ptr<UL_SBEntry> entry(new UL_SBEntry(Get, S_SENDING_A, *a->address, *this->cycles));
                localBoard->update(*a->source, entry);
                break;
            }
            case PutFullData: {
                std::shared_ptr<UL_SBEntry> entry(new UL_SBEntry(PutFullData, S_SENDING_A, *a->address, *this->cycles));
                localBoard->update(*a->source, entry);
                int beat_num = pendingA.nr_beat - pendingA.beat_cnt;
                for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
                    this->port->a.data[i - BEATSIZE * beat_num] = a->data[i];
                }
                break;
            }
            case PutPartialData: {
                std::shared_ptr<UL_SBEntry> entry(new UL_SBEntry(PutPartialData, S_SENDING_A, *a->address, *this->cycles));
                localBoard->update(*a->source, entry);
                int beat_num = pendingA.nr_beat - pendingA.beat_cnt;
                for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
                    this->port->a.data[i - BEATSIZE * beat_num] = a->data[i];
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

    Resp ULAgent::send_c(std::shared_ptr<ChnC<ReqField, EchoField, DATASIZE>> &c) {
        return OK;
    }

    void ULAgent::fire_a() {
        if (this->port->a.fire()) {
            auto chnA = this->port->a;
            bool hasData = *chnA.opcode == PutFullData || *chnA.opcode == PutPartialData;
            *chnA.valid = false;
            tlc_assert(pendingA.is_pending(), "No pending A but A fired!");
            pendingA.update();
            if (!pendingA.is_pending()) { // req A finished
                this->localBoard->query(*pendingA.info->source)->update_status(S_A_WAITING_D, *cycles);
                if (hasData) {
                    std::shared_ptr<Global_SBEntry> global_SBEntry(new Global_SBEntry());
                    global_SBEntry->pending_data = pendingA.info->data;
                    if (this->globalBoard->get().count(*pendingA.info->address) == 0) {
                        global_SBEntry->data = nullptr;
                    } else {
                        global_SBEntry->data = this->globalBoard->get()[*pendingA.info->address]->data;
                    }
                    global_SBEntry->status = Global_SBEntry::SB_PENDING;
                    this->globalBoard->update(*pendingA.info->address, global_SBEntry);
                }
            }
        }
    }

    void ULAgent::fire_b() {

    }

    void ULAgent::fire_c() {

    }

    void ULAgent::fire_d() {
        if (this->port->d.fire()) {
            auto chnD = this->port->d;
            auto info = localBoard->query(*chnD.source);
            bool hasData = *chnD.opcode == GrantData || *chnD.opcode == AccessAckData;
            tlc_assert(info->status == S_A_WAITING_D, "Status error!");
            if (pendingD.is_pending()) { // following beats
                // TODO: wrap the following assertions into a function
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
                int nr_beat = (*chnD.opcode == Grant || *chnD.opcode == AccessAck || *chnD.size <= 5) ? 0 : 1; // TODO: parameterize it
                pendingD.init(resp_d, nr_beat);
            }
            // Store data to pendingD
            if (hasData) {
                int beat_num = pendingD.nr_beat - pendingD.beat_cnt;
                for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
                    pendingD.info->data[i] = chnD.data[i - BEATSIZE * beat_num];
                }
            }
            if (!pendingD.is_pending()) {
                // ULAgent needn't care about endurance
                if (hasData) {
                    Log("[%ld] [AccessAckData] addr: %hx data: ", *cycles, info->address);
                    for(int i = 0; i < DATASIZE; i++) {
                        Dump("%02hhx", pendingD.info->data[i]);
                    }
                    Dump("\n");
                    this->globalBoard->verify(info->address, pendingD.info->data);
                } else if (*chnD.opcode == AccessAck) { // finish pending status in GlobalBoard
                    Log("[%ld] [AccessAck] addr: %hx\n", *cycles, info->address);
                    this->globalBoard->unpending(info->address);
                }
                localBoard->erase(*chnD.source);
                this->idpool.freeid(*chnD.source);
            }
        }
    }
    
    void ULAgent::fire_e() {
    }

    void ULAgent::handle_b(std::shared_ptr<ChnB> &b) {
    }
    
    void ULAgent::handle_channel() {
        fire_a();
        fire_d();
    }

    void ULAgent::update_signal() {
        *this->port->d.ready = true; // TODO: do random here
        if (pendingA.is_pending()) {
            // TODO: do delay here
            send_a(pendingA.info);
        } else {
            *this->port->a.valid = false;
        }
        // do timeout check lazily
        if (*this->cycles % TIMEOUT_INTERVAL == 0) {
            this->timeout_check();
        }
        idpool.update();
    }
    
    bool ULAgent::do_getAuto(paddr_t address) {
        if (pendingA.is_pending() || idpool.full())
            return false;
        std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> req_a(new ChnA<ReqField, EchoField, DATASIZE>());
        req_a->opcode = new uint8_t(Get);
        req_a->address = new paddr_t(address);
        req_a->size = new uint8_t(ceil(log2((double)DATASIZE)));
        req_a->mask = new uint32_t(0xffffffffUL);
        req_a->source = new uint8_t(this->idpool.getid());
        pendingA.init(req_a, 1);
        Log("[%ld] [Get] addr: %x\n", *cycles, address);
        return true;
    }

    bool ULAgent::do_get(paddr_t address, uint8_t size, uint32_t mask) {
        if (pendingA.is_pending() || idpool.full())
            return false;
        std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> req_a(new ChnA<ReqField, EchoField, DATASIZE>());
        req_a->opcode = new uint8_t(Get);
        req_a->address = new paddr_t(address);
        req_a->size = new uint8_t(size);
        req_a->mask = new uint32_t(mask);
        req_a->source = new uint8_t(this->idpool.getid());
        pendingA.init(req_a, 1);
        Log("[%ld] [Get] addr: %x size: %x\n", *cycles, address, size);
        return true;
    }
    
    bool ULAgent::do_putfulldata(uint16_t address, uint8_t data[]) {
        if (pendingA.is_pending() || idpool.full())
            return false;
        if (this->globalBoard->haskey(address) && this->globalBoard->query(address)->status == Global_SBEntry::SB_PENDING) {
            return false;
        }
        std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> req_a(new ChnA<ReqField, EchoField, DATASIZE>());
        req_a->opcode = new uint8_t(PutFullData);
        req_a->address = new paddr_t(address);
        req_a->size = new uint8_t(ceil(log2((double)DATASIZE)));
        req_a->mask = new uint32_t(0xffffffffUL);
        req_a->source = new uint8_t(this->idpool.getid());
        req_a->data = data;
        pendingA.init(req_a, DATASIZE / BEATSIZE);
        Log("[%ld] [PutFullData] addr: %x data: ", *cycles, address);
        for(int i = 0; i < DATASIZE; i++) {
            Dump("%02hhx", data[i]);
        }
        Dump("\n");
        return true;
    }

    bool ULAgent::do_putpartialdata(uint16_t address, uint8_t size, uint32_t mask, uint8_t data[]) {
        if (pendingA.is_pending() || idpool.full())
            return false;
        if (this->globalBoard->haskey(address) && this->globalBoard->query(address)->status == Global_SBEntry::SB_PENDING)
            return false;
        std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> req_a(new ChnA<ReqField, EchoField, DATASIZE>());
        req_a->opcode = new uint8_t(PutPartialData);
        req_a->address = new paddr_t(address);
        req_a->size = new uint8_t(size);
        req_a->mask = new uint32_t(mask);
        req_a->source = new uint8_t(this->idpool.getid());
        req_a->data = data;
        int nrBeat = ceil((float)pow(2, size) / (float)BEATSIZE);
        pendingA.init(req_a, nrBeat);
        Log("[%ld] [PutPartialData] addr: %x data: ", *cycles, address);
        for(int i = 0; i < DATASIZE; i++) {
            Dump("%02hhx", data[i]);
        }
        Dump("\n");
        return true;
    }
    
    void ULAgent::timeout_check() {
        if (localBoard->get().empty()) {
            return;
        }
        for (auto it = this->localBoard->get().begin(); it != this->localBoard->get().end(); it++) {
            auto value = it->second;
            if (value->status != S_INVALID && value->status != S_VALID) {
                if (*this->cycles - value->time_stamp > TIMEOUT_INTERVAL) {
                    printf("Now time:   %lu\n", *this->cycles);
                    printf("Last stamp: %lu\n", value->time_stamp);
                    printf("Status:     %d\n", value->status);
                    tlc_assert(false,  "Transaction time out");
                }
            }
        }
    }
}