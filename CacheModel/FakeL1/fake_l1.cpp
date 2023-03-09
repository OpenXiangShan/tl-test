//
// Created by zcy on 2023/2/27.
//

#include "fake_l1.h"


namespace fake_l1 {

    int shrink_gen_priv(int param) {
        using namespace tl_agent;
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

    int cap_gen_priv(int param) {
        using namespace tl_agent;
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

    FakeL1::FakeL1(GlobalBoard<paddr_t> *const gb, int id,uint64_t *cycles, uint64_t core_id, int bus_type): 
        a_idpool(GET_CA_A_ID_BEGIN(bus_type), GET_CA_A_ID_END(bus_type)),
        c_idpool(GET_CA_C_ID_BEGIN(bus_type), GET_CA_C_ID_END(bus_type)) {
        this->globalBoard = gb;
        this->core_id = core_id;
        this->id = id;
        this->bus_type = bus_type;
        this->cycles = cycles;
        this->cache_info.reset(new ScoreBoard<paddr_t, C_SBEntry>());
        this->a_mshr_info.reset(new ScoreBoard<int, C_IDEntry>());
        this->b_mshr_info.reset(new ScoreBoard<int, C_IDEntry>());
        this->c_mshr_info.reset(new ScoreBoard<int, C_IDEntry>());
        this->agent.reset(new tl_c_agent::TLCAgent(cycles, id, core_id, bus_type));
        this->agent->port.reset(new Port<ReqField, RespField, EchoField, BEATSIZE>());
        // automatically connect agetn->port to DPI-C interface when instantiate this class
        this->connect();
    }

    FakeL1::~FakeL1() {

    }

    void FakeL1::connect() {
        // tl_info will finally be connected to DPI-C interface
        // to do this, we should register tl_info into the interface
        // as FakeL1, we can simply drive agent->port signals when we want to send a transaction to DUT
        // dataflow: [agent->port] ->> [tl_info] ->> [DUT]  
        this->tl_info.reset(new tl_interface::TLInfo(core_id, bus_type));
        tl_interface::register_tlc_info(this->tl_info);
        this->tl_info->connect(this->agent->port);  
    }

    void FakeL1::timeout_check() {
        if (cache_info->get().empty()) {
            return;
        }
        for (auto it = this->cache_info->get().begin();
            it != this->cache_info->get().end(); it++) {
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

    void FakeL1::transaction_input(tl_base_agent::TLCTransaction tr) {
        using namespace tl_base_agent;
        if(local_probe(tr.addr))
            return;
        switch (tr.opcode) {
            case ACQUIRE_PERM:
                do_acquirePerm(tr.addr, tr.param, tr.alias);
                break;
            case ACQUIRE_BLOCK:
                do_acquireBlock(tr.addr, tr.param, tr.alias);
                break;
            case RELEASE_DATA:
                do_releaseData(tr.addr,tr.param,tr.data,tr.alias);
                break;
            case RELEASE_DATA_AUTO:
                do_releaseDataAuto(tr.addr, tr.alias);
                break;
            case INVALID_REQ:
                break;
            default:
                tlc_assert(false, "Invalid transaction input!");
                break;
        }
    }

    // handle channel and update cache info
    void FakeL1::handle_channel() {
        // !! Constraint: fire_e > fire_d, otherwise concurrent D/E requests will disturb the pendingE
        // ---------------------------------------------------------------------------------------
        this->agent->fire_a();
        if(this->agent->update_a()) { // cache_info will update every time there is a valid transaction being fired
            // Log("[%ld] [A fire] addr: %lx\n", *cycles, *this->agent->port->a.address);
            if (!this->agent->pendingA.is_pending()) { // req A finished
                this->cache_info->query(*this->agent->pendingA.info->address)
                    ->update_status(S_A_WAITING_D, *cycles, *this->agent->pendingA.info->alias);
            }
        }

        // ---------------------------------------------------------------------------------------
        this->agent->fire_b();
        if(this->agent->update_b()) { 
            // report request
            auto &channel_b = this->agent->port->b;
            std::string param_str = *channel_b.param==toN? "toN":(*channel_b.param==toB? "toB":(*channel_b.param==toT? "toT":"Unknown"));
            Log("[%ld] [B] [Probe %s] addr: %lx alias: %d\n", *cycles, param_str.c_str(), *channel_b.address,
                (*channel_b.alias) >> 1); 
        }


        // ---------------------------------------------------------------------------------------
        this->agent->fire_c();
        if(this->agent->update_c()) {
            auto &channel_c = this->agent->port->c;
            auto &pendingC = this->agent->pendingC;

            bool releaseHasData = *channel_c.opcode == ReleaseData;
            bool needAck = *channel_c.opcode == ReleaseData || *channel_c.opcode == Release;
            bool probeAckDataToB = *channel_c.opcode == ProbeAckData &&
                                (*channel_c.param == TtoB || *channel_c.param == BtoB);

            if (!pendingC.is_pending()) { // req C finished
                *channel_c.valid = false;
                // Log("[%ld] [C fire] addr: %lx opcode: %lx\n", *cycles, *channel_c.address,
                // *channel_c.opcode);
                auto info = this->cache_info->query(*pendingC.info->address);
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
                if (*channel_c.opcode == ProbeAckData || *channel_c.opcode == ProbeAck) {
                    std::shared_ptr<Global_SBEntry> global_SBEntry(new Global_SBEntry());
                    bool origin_entry_is_pending = false;
                    global_SBEntry->data = pendingC.info->data;
                    if (this->globalBoard->get().count(*pendingC.info->address) != 0) {
                    auto originEntry = this->globalBoard->get()[*pendingC.info->address];
                    global_SBEntry->pending_data = originEntry->pending_data;
                    if(*channel_c.opcode == ProbeAck)
                        global_SBEntry->data = originEntry->data; 
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
                    if(*channel_c.opcode == ProbeAck){
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
                if (*channel_c.opcode == ReleaseData || *channel_c.opcode == Release) {
                    info->update_pending_priviledge(shrink_gen_priv(*pendingC.info->param),
                                                    *cycles, *pendingC.info->alias);
                } else {
                    if (*channel_c.opcode == ProbeAck || *channel_c.opcode == ProbeAckData) {
                    info->update_priviledge(shrink_gen_priv(*pendingC.info->param), *cycles,
                                            *pendingC.info->alias);
                    }
                    // Log("== free == fireC %d\n", *channel_c.source);
                    this->c_idpool.freeid(*channel_c.source);
                    // Log("c_idpool free id => probeack %d\n", *channel_c.source);
                }
            }
        }

        // ---------------------------------------------------------------------------------------
        this->agent->fire_d();
        if(this->agent->update_d()) {
            auto &channel_d = this->agent->port->d;
            auto &pendingD = this->agent->pendingD;
            auto &pendingE = this->agent->pendingE;

            bool hasData = *channel_d.opcode == GrantData;
            bool grant = *channel_d.opcode == GrantData || *channel_d.opcode == Grant;
            auto addr = grant? (a_mshr_info->query(*channel_d.source)->address):(c_mshr_info->query(*channel_d.source)->address);
            auto alias = grant? (a_mshr_info->query(*channel_d.source)->alias):(c_mshr_info->query(*channel_d.source)->alias);
            auto info = cache_info->query(addr);
            auto exact_status = info->status[alias];

            if (!(exact_status == S_C_WAITING_D || exact_status == S_A_WAITING_D ||
                exact_status == S_C_WAITING_D_INTR ||
                exact_status == S_A_WAITING_D_INTR || exact_status == S_INVALID)) {
                printf("fire_d: status of cache_info is %d\n", exact_status);
                printf("addr: 0x%lx\n", addr);
                tlc_assert(false, "Status error!");
            }

            if (!pendingD.is_pending()) {
                uint8_t p = *(channel_d.param);
                std::string param_str = p==toT? "toT":(p==toB? "toB":(p==toN? "toN":"Unknown"));
                switch (*channel_d.opcode) {
                    case GrantData: {
                        Log("[%ld] [D] [GrantData %s] addr: %lx source: %d sink: %d, data: ", *cycles, param_str.c_str(), addr, *(channel_d.source), *(channel_d.sink));
                        for (int i = 0; i < DATASIZE; i++) {
                            Dump("%02hhx", pendingD.info->data[DATASIZE - 1 - i]);
                        }
                        Dump("\n");
                        this->globalBoard->verify(addr, pendingD.info->data);
                        info->update_dirty(*channel_d.dirty, alias);
                        this->a_idpool.freeid(*channel_d.source);
                        // Log("a_idpool free id => GrantData %d\n",*channel_d.source);
                        a_mshr_info->erase(*channel_d.source);
                        break;
                    }
                    case Grant: {
                        Log("[%ld] [D] [Grant %s] addr: %lx source: %d sink: %d\n", *cycles, param_str.c_str(), addr, *(channel_d.source), *(channel_d.sink));
                        info->update_dirty(*channel_d.dirty, alias);
                        this->a_idpool.freeid(*channel_d.source);
                        // Log("a_idpool free id => Grant %d\n",*channel_d.source);
                        a_mshr_info->erase(*channel_d.source);
                        break;
                    }
                    case ReleaseAck: {
                        Log("[%ld] [D] [ReleaseAck] addr: %lx source: %d \n", *cycles, addr, *(channel_d.source));
                        if (exact_status == S_C_WAITING_D) {
                            info->update_status(S_INVALID, *cycles, alias);
                            info->update_dirty(0, alias);
                        } else {
                            tlc_assert(exact_status == S_C_WAITING_D_INTR, "Status error!");
                            info->update_status(S_SENDING_C, *cycles, alias);
                        }
                        info->unpending_priviledge(*cycles, alias);
                        this->globalBoard->unpending(addr);
                        this->c_idpool.freeid(*channel_d.source);
                        // Log("c_idpool free id => ReleaseAck %d\n", *channel_d.source);
                        c_mshr_info->erase(*channel_d.source);
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
                    req_e->sink.reset(new uint32_t(*channel_d.sink));
                    req_e->addr.reset(new paddr_t(addr));
                    req_e->alias.reset(new uint8_t(alias));
                    if (pendingE.is_pending()) {
                        Log("New E request when E is pending!\n");
                        tlc_assert(false, "E is pending!");
                    }
                    pendingE.init(req_e, 1);

                    info->update_status(S_SENDING_E, *cycles, alias);
                    info->update_priviledge(cap_gen_priv(*channel_d.param), *cycles, alias);
                    Log("[%ld] [E] [GrantAck] addr: %lx sink: %d\n", *cycles, *(req_e->addr), *(req_e->sink));
                }
                // Log("== free == fireD %d\n", *channel_d.source);
            }
        }

        // ---------------------------------------------------------------------------------------
        this->agent->fire_e();
        if(this->agent->update_e()) {
            auto &pendingE = this->agent->pendingE;
            auto info = cache_info->query(*pendingE.info->addr);
            info->update_status(S_VALID, *cycles, *pendingE.info->alias);
        }

    }

    void FakeL1::update_signal() {
        auto &channel_d = this->agent->port->d;
        auto &channel_b = this->agent->port->b;

        auto &pendingE = this->agent->pendingE;
        auto &pendingB = this->agent->pendingB;
        
        *channel_d.ready = !(pendingE.is_pending());
        *channel_b.ready = !(pendingB.is_pending());

        if (this->agent->pendingA.is_pending()) {
            // TODO: do delay here (random delay)
            // cache info update
            using namespace tl_agent;
            auto a = this->agent->pendingA.info;
            switch (*a->opcode) {
                case AcquireBlock: {
                    std::shared_ptr<C_IDEntry> idmap_entry(
                        new C_IDEntry(*a->address, *a->alias));
                    a_mshr_info->update(*a->source, idmap_entry);

                    if (cache_info->haskey(*a->address)) {
                        cache_info->query(*a->address)
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
                        cache_info->update(*a->address, entry);
                    }
                    break;
                }
                case AcquirePerm: {
                    std::shared_ptr<C_IDEntry> idmap_entry(
                        new C_IDEntry(*a->address, *a->alias));
                    a_mshr_info->update(*a->source, idmap_entry);
                    if (cache_info->haskey(*a->address)) {
                        cache_info->query(*a->address)
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
                        cache_info->update(*a->address, entry);
                    }
                    break;
                }
                default:
                    tlc_assert(false, "Unknown opcode for channel A!");
            }

            // update channel signals
            this->agent->send_a(a);
        }
        if (this->agent->pendingB.is_pending()) {
            // cache info update
            auto &pendingC = this->agent->pendingC;
            auto &pendingB = this->agent->pendingB;
            bool busy = false;

            if (pendingC.is_pending()) {
                Log("[info] B wanna pendingC\n");
                // return;
                goto out;
                // busy = true;
            }
            if (this->c_idpool.full()) {
                Log("[info] id pool full\n");
                // return;
                goto out;
                // busy = true;
            }

            auto b = this->agent->pendingB.info;
            tlc_assert(cache_info->haskey(*b->address), "Probe an non-exist block!");

            auto info = cache_info->query(*b->address);
            auto exact_status = info->status[*b->alias];
            auto exact_privilege = info->privilege[*b->alias];
            tlc_assert(exact_status != S_SENDING_C,
                "handle_b should be mutual exclusive with pendingC!");
            if (exact_status == S_C_WAITING_D) {
                // Probe waits for releaseAck
                // return;
                goto out;
                // busy = true;
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
out:
        if (this->agent->pendingC.is_pending()) {
            using namespace tl_agent;
            auto c = this->agent->pendingC.info;
            switch (*c->opcode) {
                case ReleaseData: {
                    std::shared_ptr<C_IDEntry> idmap_entry(
                        new C_IDEntry(*c->address, *c->alias));
                    c_mshr_info->update(*c->source, idmap_entry);

                    if (cache_info->haskey(*c->address)) {
                    cache_info->query(*c->address)
                        ->update_status(S_SENDING_C, *cycles, *c->alias);
                    } else {
                    tlc_assert(false, "cache_info key not found!");
                    }
                    int beat_num = this->agent->pendingC.nr_beat - this->agent->pendingC.beat_cnt;
                    for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
                        // !!!!!
                        this->agent->port->c.data[i - BEATSIZE * beat_num] = c->data[i];
                    }
                    break;
                }
                case ProbeAckData: {
                    std::shared_ptr<C_IDEntry> idmap_entry(
                        new C_IDEntry(*c->address, *c->alias));
                    b_mshr_info->update(*c->source, idmap_entry);

                    if (cache_info->haskey(*c->address)) {
                    // TODO: What if this is an interrupted probe?
                    cache_info->query(*c->address)
                        ->update_status(S_SENDING_C, *cycles, *c->alias);
                    } else {
                    tlc_assert(false, "cache_info key not found!");
                    }
                    int beat_num = this->agent->pendingC.nr_beat - this->agent->pendingC.beat_cnt;
                    for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
                        // !!!
                        this->agent->port->c.data[i - BEATSIZE * beat_num] = c->data[i];
                    }
                    break;
                }
                case ProbeAck: {
                    std::shared_ptr<C_IDEntry> idmap_entry(
                        new C_IDEntry(*c->address, *c->alias));
                    b_mshr_info->update(*c->source, idmap_entry);
                    // tlc_assert(*c->param == NtoN, "Now probeAck only supports NtoN");
                    if (cache_info->haskey(*c->address)) {
                        auto item = cache_info->query(*c->address);
                        switch(item->status[*c->alias]){
                            case S_C_WAITING_D: item->update_status(S_C_WAITING_D_INTR, *cycles, *c->alias);break;
                            case S_A_WAITING_D: item->update_status(S_A_WAITING_D_INTR, *cycles, *c->alias);break;
                            case S_C_WAITING_D_INTR: break;
                            case S_A_WAITING_D_INTR: break;
                            default: item->update_status(S_SENDING_C, *cycles, *c->alias);break;
                        }
                    } else {
                        tlc_assert(false, "cache_info key not found!");
                    }
                    break;
                }
                default:
                    tlc_assert(false, "Unknown opcode for channel C!");
            }
            this->agent->send_c(c);
        }
        if (this->agent->pendingE.is_pending()) {
            this->agent->send_e(this->agent->pendingE.info);
        }

        // do timeout check lazily
        if (*this->cycles % TIMEOUT_INTERVAL == 0) {
            this->timeout_check();
        }

        this->a_idpool.update();
        this->c_idpool.update();
    }

    bool FakeL1::do_acquireBlock(paddr_t address, int param, int alias) {
        using namespace tl_agent;
        if(bus_type == ICACHE_BUS_TYPE){
            tlc_assert(param == NtoB, "ICache can only acquire block with NtoB!\n");
        }
        if (this->agent->pendingA.is_pending() || this->agent->pendingB.is_pending() || a_idpool.full())
            return false;
        if (cache_info->haskey(address)) { // check whether this transaction is legal
            auto entry = cache_info->query(address);
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
        this->agent->pendingA.init(req_a, 1);
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

    bool FakeL1::do_acquirePerm(paddr_t address, int param, int alias) {
        using namespace tl_agent;
        if(bus_type == ICACHE_BUS_TYPE){
            tlc_assert(param == NtoB, "ICache can only acquire block with NtoB!\n");
        }
        if (this->agent->pendingA.is_pending() || this->agent->pendingB.is_pending() || a_idpool.full())
            return false;
        if (cache_info->haskey(address)) {
            auto entry = cache_info->query(address);
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
        this->agent->pendingA.init(req_a, 1);
        std::string param_str = param == NtoB? "NtoB":(param == NtoT? "NtoT":(param == BtoT?"BtoT":"Unknown"));
        Log("[%ld] [A] [AcquirePerm %s] addr: %lx source: %d alias: %d\n", *cycles, param_str.c_str(), address, *(req_a->source), alias);
        return true;
    }

    bool FakeL1::do_releaseData(paddr_t address, int param, std::shared_ptr<uint8_t[]>data, int alias) {
        using namespace tl_agent;
        if (this->agent->pendingC.is_pending() || this->agent->pendingB.is_pending() || c_idpool.full() ||
            !cache_info->haskey(address))
            return false;
        // TODO: checkout pendingA
        // TODO: checkout pendingB - give way?
        auto entry = cache_info->query(address);
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
        this->agent->pendingC.init(req_c, DATASIZE / BEATSIZE);
        Log("[%ld] [C] [ReleaseData] addr: %lx, data: ", *cycles, address);
        for (int i = 0; i < DATASIZE; i++) {
            Dump("%02hhx", data[DATASIZE - 1 - i]);
        }
        Dump("\n");
        return true;
    }

    bool FakeL1::do_releaseDataAuto(paddr_t address, int alias) {
        using namespace tl_agent;
        if (this->agent->pendingC.is_pending() || this->agent->pendingB.is_pending() || c_idpool.full() ||
            !cache_info->haskey(address))
            return false;
        // TODO: checkout pendingA
        // TODO: checkout pendingB - give way?
        auto entry = cache_info->query(address);
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
        this->agent->pendingC.init(req_c, DATASIZE / BEATSIZE);
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

    bool FakeL1::local_probe(paddr_t address) {
        // can exist in either cache_info, pendingA or pendingC
        // TODO: how about unaligned address?
        if (!cache_info->haskey(address))
            return false;
        // block inner Acquire
        if (this->agent->pendingA.is_pending() && *this->agent->pendingA.info->address == address)
            return true;
        // block inner Release
        if (this->agent->pendingC.is_pending() && *this->agent->pendingC.info->address == address)
            return true;
        // we already have this block
        std::shared_ptr<tl_agent::C_SBEntry> entry = cache_info->query(address);
        for (int i = 0; i < 4; i++) {
            if (entry->status[i] != S_INVALID && entry->status[i] != S_VALID)
            return true;
        }
        return false;
    }

}
