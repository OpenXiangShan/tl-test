//
// Created by zcy on 2023/3/2.
//

#include "fake_ptw.h"

namespace fake_ptw {
    
    inline paddr_t align_addr(paddr_t addr) { return addr & 0xFFFFFFC0; }

    FakePTW::FakePTW(GlobalBoard<paddr_t> *const gb, int id,uint64_t *cycles, uint64_t core_id, int bus_type): 
        a_idpool(GET_UA_A_ID_BEGIN(bus_type), GET_UA_A_ID_END(bus_type)) {
        using namespace tl_interface;
        this->core_id = core_id;
        this->bus_type = bus_type;
        this->globalBoard = gb;
        this->id = id;
        this->cycles = cycles;
        this->localBoard = new ScoreBoard<int, UL_SBEntry>();
        this->agent.reset(new tl_ul_agent::TLULAgent(cycles, id, core_id, bus_type));
        this->agent->port.reset(new Port<ReqField, RespField, EchoField, BEATSIZE>());
        this->connect();
    }

    std::shared_ptr<tl_interface::TLInfo> FakePTW::get_info(){
        return this->tl_info;
    }

    void FakePTW::connect() {
        this->tl_info.reset(new tl_interface::TLInfo(core_id,bus_type));
        tl_interface::register_tlu_info(this->tl_info);
        this->tl_info->connect(this->agent->port);
    }

    void FakePTW::transaction_input(tl_base_agent::TLCTransaction tr) {
        using namespace tl_base_agent;
        if(local_probe(tr.addr)) 
            return;
        switch (tr.opcode)
        {
            case GET_AUTO:
                do_getAuto(tr.addr);
                break;
            case GET:
                do_get(tr.addr, tr.size, tr.mask);
                break;
            case PUT_FULL_DATA:
                do_putfulldata(tr.addr, tr.data);
                break;
            case PUT_PARTIAL_DATA:
                do_putpartialdata(tr.addr, tr.offset, tr.size, tr.mask, tr.data);
                break;
            default:
                break;
        }
    }

    void FakePTW::handle_channel() {
        this->agent->fire_a();
        if(this->agent->update_a()) {
            auto &pendingA = this->agent->pendingA;
            auto &channel_a = this->agent->port->a;
            bool hasData = *channel_a.opcode == PutFullData || *channel_a.opcode == PutPartialData;
            if (!pendingA.is_pending()) { // req A finished
                this->localBoard->query(*pendingA.info->source)
                    ->update_status(S_A_WAITING_D, *cycles);
                if (hasData) {
                    std::shared_ptr<Global_SBEntry> global_SBEntry(new Global_SBEntry());
                    global_SBEntry->pending_data = pendingA.info->data;
                    global_SBEntry->mask =
                        (*pendingA.info->opcode == PutFullData && *pendingA.info->size == 6)
                            ? FULLMASK
                            : *pendingA.info->mask;
                    if (this->globalBoard->get().count(*pendingA.info->address) == 0) {
                    global_SBEntry->data = nullptr;
                    } else {
                    global_SBEntry->data =
                        this->globalBoard->get()[*pendingA.info->address]->data;
                    }
                    global_SBEntry->status = Global_SBEntry::SB_PENDING;
                    this->globalBoard->update(*pendingA.info->address, global_SBEntry);
                }
            }
        }

        this->agent->fire_d();
        if(this->agent->update_d()) {
            auto &channel_d = this->agent->port->d;
            auto &pendingD = this->agent->pendingD;

            tlc_assert(localBoard->haskey(*channel_d.source),
                    "SourceID in chnD is invalid!");

            auto info = localBoard->query(*channel_d.source);
            bool hasData = *channel_d.opcode == GrantData || *channel_d.opcode == AccessAckData;
            tlc_assert(info->status == S_A_WAITING_D, "Status error!");

            if (pendingD.is_pending()) { // following beats
                // TODO: wrap the following assertions into a function
                tlc_assert(*channel_d.opcode == *pendingD.info->opcode,
                            "Opcode mismatch among beats!");
                tlc_assert(*channel_d.param == *pendingD.info->param,
                            "Param mismatch among beats!");
                tlc_assert(*channel_d.source == *pendingD.info->source,
                            "Source mismatch among beats!");
                pendingD.update();
            } else { // new D resp
                std::shared_ptr<ChnD<RespField, EchoField, DATASIZE> >resp_d(new ChnD<RespField, EchoField, DATASIZE>());
                resp_d->opcode.reset(new uint8_t(*channel_d.opcode));
                resp_d->param.reset(new uint8_t(*channel_d.param));
                resp_d->source.reset(new uint32_t(*channel_d.source));
                resp_d->data.reset(hasData ? new uint8_t[DATASIZE] : nullptr);
                int nr_beat = (*channel_d.opcode == Grant || *channel_d.opcode == AccessAck ||
                                *channel_d.size <= 5)
                                    ? 0
                                    : 1; // TODO: parameterize it
                pendingD.init(resp_d, nr_beat);
            }
            if (hasData) {
                int beat_num = pendingD.nr_beat - pendingD.beat_cnt;
                for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
                    pendingD.info->data[i] = channel_d.data[i - BEATSIZE * beat_num];
                }
            }
            if (!pendingD.is_pending()) {
                // ULAgent needn't care about endurance
                if (hasData) {
                    Log("[%ld] [D] [AccessAckData] addr: %lx source: %d data: ", *cycles, info->address, *(channel_d.source));
                    for (int i = 0; i < DATASIZE; i++) {
                    Dump("%02hhx", pendingD.info->data[DATASIZE - 1 - i]);
                    }
                    Dump("\n");
                    this->globalBoard->verify(info->address, pendingD.info->data);
                } else if (*channel_d.opcode ==
                            AccessAck) { // finish pending status in GlobalBoard
                    Log("[%ld] [D] [AccessAck] addr: %lx source: %d\n", *cycles, info->address, *(channel_d.source));
                    this->globalBoard->unpending(info->address);
                }
                localBoard->erase(*channel_d.source);
                this->a_idpool.freeid(*channel_d.source);
            }
        }
    }

    void FakePTW::update_signal() {
        auto &channel_d = this->agent->port->d;
        
        *channel_d.ready = true; // TODO: do random here
        if (this->agent->pendingA.is_pending()) {
            auto &pendingA = this->agent->pendingA;
            auto &channel_a = this->agent->port->a;
            auto a = pendingA.info;
            // TODO: do delay here
            switch (*a->opcode) {
                case Get: {
                    std::shared_ptr<UL_SBEntry> entry(
                        new UL_SBEntry(Get, S_SENDING_A, *a->address, *this->cycles));
                    localBoard->update(*a->source, entry);
                    break;
                }
                case PutFullData: {
                    std::shared_ptr<UL_SBEntry> entry(new UL_SBEntry(
                        PutFullData, S_SENDING_A, align_addr(*a->address), *this->cycles));
                    localBoard->update(*a->source, entry);
                    int beat_num = pendingA.nr_beat - pendingA.beat_cnt;
                    
                    for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
                        channel_a.data[i - BEATSIZE * beat_num] = a->data[i];
                    }
                    break;
                }
                case PutPartialData: {
                    std::shared_ptr<UL_SBEntry> entry(new UL_SBEntry(
                        PutPartialData, S_SENDING_A, align_addr(*a->address), *this->cycles));
                    localBoard->update(*a->source, entry);
                    int beat_num = pendingA.nr_beat - pendingA.beat_cnt;
                    for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
                        channel_a.data[i - BEATSIZE * beat_num] = a->data[i];
                    }
                    break;
                }
                default:
                    tlc_assert(false, "Unknown opcode for channel A!");
            }
            this->agent->send_a(a);
        } else {
            *channel_d.valid = false;
        }
        // do timeout check lazily
        if (*this->cycles % TIMEOUT_INTERVAL == 0) {
            this->timeout_check();
        }
        a_idpool.update();
    }

    bool FakePTW::do_getAuto(paddr_t address) {
        auto &pendingA = this->agent->pendingA;
        if (pendingA.is_pending() || a_idpool.full())
            return false;
        std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE> >req_a(new ChnA<ReqField, EchoField, DATASIZE>());
        req_a->opcode.reset(new uint8_t(Get));
        req_a->address.reset(new paddr_t(address));
        req_a->size.reset(new uint8_t(ceil(log2((double)DATASIZE))));
        req_a->mask.reset(new uint32_t(0xffffffffUL));
        req_a->source.reset(new uint32_t(this->a_idpool.getid()));
        pendingA.init(req_a, 1);
        Log("[%ld] [A] [Get] addr: %lx source: %d\n", *cycles, address, *(req_a->source));
        return true;
    }

    bool FakePTW::do_get(paddr_t address, uint8_t size, uint32_t mask) {
        auto &pendingA = this->agent->pendingA;
        if (pendingA.is_pending() || a_idpool.full())
            return false;
        std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE> >req_a(new ChnA<ReqField, EchoField, DATASIZE>());
        req_a->opcode.reset(new uint8_t(Get));
        req_a->address.reset(new paddr_t(address));
        req_a->size.reset(new uint8_t(size));
        req_a->mask.reset(new uint32_t(mask));
        req_a->source.reset(new uint32_t(this->a_idpool.getid()));
        pendingA.init(req_a, 1);
        Log("[%ld] [A] [Get] addr: %lx size: %x\n", *cycles, address, size);
        return true;
    }

    bool FakePTW::do_putfulldata(paddr_t address, std::shared_ptr<uint8_t[]> data) {
        auto &pendingA = this->agent->pendingA;
        if (pendingA.is_pending() || a_idpool.full())
            return false;
        if (this->globalBoard->haskey(address) &&
            this->globalBoard->query(address)->status == Global_SBEntry::SB_PENDING) {
            return false;
        }
        std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE> >req_a(new ChnA<ReqField, EchoField, DATASIZE>());
        req_a->opcode.reset(new uint8_t(PutFullData));
        req_a->address.reset(new paddr_t(address));
        req_a->size.reset(new uint8_t(ceil(log2((double)DATASIZE))));
        req_a->mask.reset(new uint32_t(0xffffffffUL));
        req_a->source.reset(new uint32_t(this->a_idpool.getid()));
        req_a->data = data;
        pendingA.init(req_a, DATASIZE / BEATSIZE);
        Log("[%ld] [A] [PutFullData] addr: %lx source: %d data: ", *cycles, address, *(req_a->source));
        for (int i = 0; i < DATASIZE; i++) {
            Dump("%02hhx", data[DATASIZE - 1 - i]);
        }
        Dump("\n");
        return true;
    }

    bool FakePTW::do_putpartialdata(paddr_t address, uint8_t offset, uint8_t size, uint32_t mask,
                            std::shared_ptr<uint8_t[]> data) {
        auto &pendingA = this->agent->pendingA;
        if (pendingA.is_pending() || a_idpool.full())
            return false;
        if (this->globalBoard->haskey(address) &&
            this->globalBoard->query(address)->status == Global_SBEntry::SB_PENDING)
            return false;
        std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE> >req_a(new ChnA<ReqField, EchoField, DATASIZE>());
        req_a->opcode.reset(new uint8_t(PutPartialData));
        req_a->address.reset(new paddr_t(address + offset));
        req_a->size.reset(new uint8_t(size));
        req_a->mask.reset(new uint32_t(mask));
        req_a->source.reset(new uint32_t(this->a_idpool.getid()));
        req_a->data = data;
        int nrBeat = ceil((float)pow(2, size) / (float)BEATSIZE);
        pendingA.init(req_a, nrBeat);
        Log("[%ld] [PutPartialData] addr: %lx source:%d size:%d mask:%x data: ", *cycles, address + offset, *(req_a->source), size, mask);
        int data_begin = offset;
        int data_end = (1 << size) + offset;
        for (int i = data_begin; i < data_end; i++) {
            Dump("%02hhx", data[data_end - 1 - i]);
        }
        Dump("\n");
        return true;
    }

    void FakePTW::timeout_check() {
        if (localBoard->get().empty()) {
            return;
        }
        for (auto it = this->localBoard->get().begin();
            it != this->localBoard->get().end(); it++) {
            auto value = it->second;
            if (value->status != S_INVALID && value->status != S_VALID) {
            if (*this->cycles - value->time_stamp > TIMEOUT_INTERVAL) {
                printf("Now time:   %lu\n", *this->cycles);
                printf("Last stamp: %lu\n", value->time_stamp);
                printf("Status:     %d\n", value->status);
                printf("Address:    %lx\n", value->address);
                tlc_assert(false, "Transaction time out");
            }
            }
        }
    }

    bool FakePTW::local_probe(paddr_t address) {
        auto &pendingA = this->agent->pendingA;
        // can exist in either localboard or pendingA
        // TODO: how about unaligned address?
        if (pendingA.is_pending() && *pendingA.info->address == address)
            return true;
        for (int i = 0; i < NR_SOURCEID; i++) {
            if (localBoard->haskey(i)) {
                std::shared_ptr<tl_agent::UL_SBEntry> entry = localBoard->query(i);
                if (entry->address == address && entry->status != S_INVALID &&
                    entry->status != S_VALID) {
                    return true;
                }
            }
        }
        return false;
    }

}