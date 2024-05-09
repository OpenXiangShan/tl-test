//
// Created by wkf on 2021/11/2.
//

#include <cstring>
#include <memory>

#include "BaseAgent.h"
#include "Bundle.h"
#include "TLEnum.h"
#include "CAgent.h"


#define CAGENT_NO_ALIAS_ACQUIRE         1


#ifndef CAGENT_NO_ALIAS_ACQUIRE
#   define CAGENT_NO_ALIAS_ACQUIRE      0
#endif


namespace tl_agent {

    int capGenPriv(TLLocalContext* ctx, int param) {
        switch (param) {
            case toT: return TIP;
            case toB: return BRANCH;
            case toN: return INVALID;
            default:
                tlc_assert(false, ctx, "Invalid param!");
        }
    }

    int shrinkGenPriv(TLLocalContext* ctx, int param) {
        switch (param) {
            case TtoT: return TIP;
            case BtoB:
            case TtoB: return BRANCH;
            case TtoN:
            case BtoN:
            case NtoN: return INVALID;
            default:
                tlc_assert(false, ctx, "Invalid param!");
        }
    }

    CAgent::CAgent(GlobalBoard<paddr_t> *const gb, int sysId, unsigned int seed, uint64_t *cycles) noexcept :
        BaseAgent(sysId, seed), pendingA(), pendingB(), pendingC(), pendingD(), pendingE(), probeIDpool(NR_SOURCEID, NR_SOURCEID+1)
    {
        this->globalBoard = gb;
        this->cycles = cycles;
        this->localBoard = new LocalScoreBoard();
        this->idMap = new IDMapScoreBoard();
    }

    CAgent::~CAgent() noexcept
    {
        delete this->localBoard;
        delete this->idMap;
    }

    uint64_t CAgent::cycle() const noexcept
    {
        return *cycles;
    }

    Resp CAgent::send_a(std::shared_ptr<BundleChannelA<ReqField, EchoField, DATASIZE>> &a) {
        switch (a->opcode) {
            case AcquireBlock: {
                auto idmap_entry = std::make_shared<C_IDEntry>(a->address, a->alias);
                idMap->update(this, a->source, idmap_entry);

                if (localBoard->haskey(a->address)) {
                    localBoard->query(this, a->address)->update_status(this, S_SENDING_A, a->alias);
                } else {
                    int statuses[4] = {S_INVALID};
                    int privileges[4] = {INVALID};
                    for (int i = 0; i < 4; i++) {
                        if (a->alias == i) {
                            statuses[i] = S_SENDING_A;
                        }
                    }
                    // Set pending as INVALID
                    auto entry = std::make_shared<C_SBEntry>(this, statuses, privileges);
                    localBoard->update(this, a->address, entry);
                }
                break;
            }
            case AcquirePerm: {
                auto idmap_entry = std::make_shared<C_IDEntry>(a->address, a->alias);
                idMap->update(this, a->source, idmap_entry);
                if (localBoard->haskey(a->address)) {
                    localBoard->query(this, a->address)->update_status(this, S_SENDING_A, a->alias);
                } else {
                    int statuses[4] = {S_INVALID};
                    int privileges[4] = {INVALID};
                    for (int i = 0; i < 4; i++) {
                        if (a->alias == i) {
                          statuses[i] = S_SENDING_A;
                        }
                    }
                    // Set pending as INVALID
                    auto entry = std::make_shared<C_SBEntry>(this, statuses, privileges);
                    localBoard->update(this, a->address, entry);
                }
                break;
            }
            default:
                tlc_assert(false, this, "Unknown opcode for channel A!");
        }
        this->port->a.opcode   = a->opcode;
        this->port->a.address  = a->address;
        this->port->a.size     = a->size;
        this->port->a.param    = a->param;
        this->port->a.mask     = a->mask;
        this->port->a.source   = a->source;
        this->port->a.alias    = a->alias;
        this->port->a.valid    = true;
        return OK;
    }

    void CAgent::handle_b(std::shared_ptr<BundleChannelB> &b) {
        if (pendingC.is_pending()) {
            Log(this, Append("B wanna pendingC\n"));
            return;
        }
        if (this->probeIDpool.full()) {
            Log(this, Append("[info] id pool full\n"));
            return;
        }

        tlc_assert(localBoard->haskey(b->address), this, "Probe an non-exist block!");

        auto info = localBoard->query(this, b->address);
        auto exact_status = info->status[b->alias];
        auto exact_privilege = info->privilege[b->alias];
        tlc_assert(exact_status != S_SENDING_C, this, "handle_b should be mutual exclusive with pendingC!");
        if (exact_status == S_C_WAITING_D) {
            // Probe waits for releaseAck
            return;
        }
        auto req_c = std::make_shared<BundleChannelC<ReqField, EchoField, DATASIZE>>();
        req_c->address  = b->address;
        req_c->size     = b->size;
        req_c->source   = this->probeIDpool.getid();
        req_c->dirty    = 1;
        req_c->alias    = b->alias;
        // Log("== id == handleB %d\n", *req_c->source);
        Log(this, ShowBase().Hex().Append("Accepting over Probe to ProbeAck: ", uint64_t(b->source), " -> ", uint64_t(req_c->source)).EndLine());
        if (exact_status == S_SENDING_A || exact_status == S_INVALID || exact_status == S_A_WAITING_D) {
            Log(this, Append("Probe an non-exist block, status: ", StatusToString(exact_status)).EndLine());
            req_c->opcode   = ProbeAck;
            req_c->param    = NtoN;
            pendingC.init(req_c, 1);
            Log(this, Append("[ProbeAck NtoN] ")
                .Hex().ShowBase().Append("addr: ", b->address, ", alias: ", uint64_t(b->alias)).EndLine());
        } else {
            int dirty = (exact_privilege == TIP) && (info->dirty[b->alias] || CAGENT_RAND64(this, "CAgent") % 3);
            // When should we probeAck with data? request need_data or dirty itself
            req_c->opcode = (dirty || b->needdata) ? ProbeAckData : ProbeAck;
            if (b->param == toB) {
                switch (exact_privilege) {
                    case TIP:    req_c->param = TtoB; break;
                    case BRANCH: req_c->param = BtoB; break;
                    default: tlc_assert(false, this, "Try to probe toB an invalid block!");
                }
            } else if (b->param == toN) {
                switch (exact_privilege) {
                    case TIP:    req_c->param = TtoN; break;
                    case BRANCH: req_c->param = BtoN; break;
                    default: tlc_assert(false, this, "Try to probe toB an invalid block!");
                }
            }
            if (!globalBoard->haskey(b->address)) {
                // want to probe an all-zero block which does not exist in global board
                Log(this, Append("probeAck Data all-zero\n"));
                std::memset((req_c->data = make_shared_tldata<DATASIZE>())->data, 0, DATASIZE);
            } else {
                if (req_c->opcode == ProbeAckData && req_c->param != BtoN) {
                    /* NOTICE: Random procedure could be better in C++ stdlib.
                               For data & seed compatibility, legacy code is preserved. The same below.
                    */
                    req_c->data = make_shared_tldata<DATASIZE>();
                    for (int i = 0; i < DATASIZE; i++) {
                      req_c->data->data[i] = (uint8_t)CAGENT_RAND64(this, "CAgent");
                    }

#                   ifdef CAGENT_DEBUG
                        Debug(this, Append("handle_b(): randomized data: "));
                        DebugEx(data_dump_embedded<DATASIZE>(req_c->data->data));
                        DebugEx(std::cout << std::endl);
#                   endif

                } else {
                    std::memcpy(
                        (req_c->data = make_shared_tldata<DATASIZE>())->data, 
                        globalBoard->query(this, b->address)->data->data, 
                        DATASIZE);

#                   ifdef CAGENT_DEBUG
                        Debug(this, Append("handle_b(): fetched scoreboard data: "));
                        DebugEx(data_dump_embedded<DATASIZE>(req_c->data->data));
                        DebugEx(std::cout << std::endl);
#                   endif
                }
            }
            if (req_c->opcode == ProbeAckData) {
                pendingC.init(req_c, DATASIZE / BEATSIZE);
            } else {
                pendingC.init(req_c, 1);
            }

            Log(this, Append("[ProbeAck", req_c->opcode == ProbeAckData ? "Data" : "", " ", 
                        ProbeAckParamToString(req_c->param), "] ")
                    .Hex().ShowBase().Append("source: ", uint64_t(req_c->source), ", addr: ", b->address, ", alias: ", uint64_t(b->alias), ", data: "));

            tlc_assert(req_c->param == TtoN
                    || req_c->param == TtoB
                    || req_c->param == NtoN
                    || req_c->param == BtoN
                    || req_c->param == BtoB, 
                this, 
                Gravity::StringAppender("Not permitted req_c param: ", ProbeAckParamToString(req_c->param)).ToString());

            if (req_c->opcode == ProbeAckData) {
                LogEx(data_dump_embedded<DATASIZE>(req_c->data->data));
            } else {
                LogEx(std::cout << "no data");
            }
            LogEx(std::cout << std::endl);
        }
        pendingB.update(this);
    }

    Resp CAgent::send_c(std::shared_ptr<BundleChannelC<ReqField, EchoField, DATASIZE>> &c) {
        switch (c->opcode) {
            case ReleaseData: {
                std::shared_ptr<C_IDEntry> idmap_entry(new C_IDEntry(c->address, c->alias));
                idMap->update(this, c->source, idmap_entry);

                if (localBoard->haskey(c->address)) {
                    localBoard->query(this, c->address)->update_status(this, S_SENDING_C, c->alias);
                } else {
                    tlc_assert(false, this, "Localboard key not found!");
                }
                int beat_num = pendingC.nr_beat - pendingC.beat_cnt;
                /*
                for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
                    this->port->c.data[i - BEATSIZE * beat_num] = c->data[i];
                }
                */
                std::memcpy(this->port->c.data->data, (uint8_t*)(c->data->data) + (BEATSIZE * beat_num), BEATSIZE);

#               ifdef CAGENT_DEBUG
                    Debug(this, Hex().ShowBase()
                        .Append("[CAgent] channel C presenting: ReleaseData: address = ", c->address)
                        .Append(", data: "));
                    DebugEx(data_dump_embedded<BEATSIZE>(this->port->c.data->data));
                    DebugEx(std::cout << std::endl);
#               endif

                break;
            }
            case ProbeAckData: {
                std::shared_ptr<C_IDEntry> idmap_entry(new C_IDEntry(c->address, c->alias));
                idMap->update(this, c->source, idmap_entry);

                if (localBoard->haskey(c->address)) {
                    // TODO: What if this is an interrupted probe?
                    localBoard->query(this, c->address)->update_status(this, S_SENDING_C, c->alias);
                } else {
                    tlc_assert(false, this, "Localboard key not found!");
                }
                int beat_num = pendingC.nr_beat - pendingC.beat_cnt;
                /*
                for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
                    this->port->c.data[i - BEATSIZE * beat_num] = c->data[i];
                }
                */
                std::memcpy(this->port->c.data->data, (uint8_t*)(c->data->data) + (BEATSIZE * beat_num), BEATSIZE);

#               ifdef CAGENT_DEBUG
                    Debug(this, Hex().ShowBase()
                        .Append("[CAgent] channel C presenting: ProbeAckData: address = ", c->address)
                        .Append(", data: "));
                    DebugEx(data_dump_embedded<BEATSIZE>(this->port->c.data->data));
                    DebugEx(std::cout << std::endl);
#               endif

                break;
            }
            case ProbeAck: {
                std::shared_ptr<C_IDEntry> idmap_entry(new C_IDEntry(c->address, c->alias));
                idMap->update(this, c->source, idmap_entry);
                // tlc_assert(*c->param == NtoN, "Now probeAck only supports NtoN");
                if (localBoard->haskey(c->address)) {
                    auto item = localBoard->query(this, c->address);
                    if (item->status[c->alias] == S_C_WAITING_D) {
                        item->update_status(this, S_C_WAITING_D_INTR, c->alias);
                    } else if (item->status[c->alias] == S_A_WAITING_D) {
                        item->update_status(this, S_A_WAITING_D_INTR, c->alias);
                    } else {
                        item->update_status(this, S_SENDING_C, c->alias);
                    }
                } else {
                    tlc_assert(false, this, "Localboard key not found!");
                }
                break;
            }
            default:
                tlc_assert(false, this, "Unknown opcode for channel C!");
        }
        this->port->c.opcode   = c->opcode;
        this->port->c.param    = c->param;
        this->port->c.address  = c->address;
        this->port->c.size     = c->size;
        this->port->c.source   = c->source;
        // this->port->c.dirty = c->dirty;
        this->port->c.valid    = true;
        return OK;
    }

    Resp CAgent::send_e(std::shared_ptr<BundleChannelE> &e) {
        this->port->e.sink = e->sink;
        this->port->e.valid = true;
        return OK;
    }

    void CAgent::fire_a() {
        if (this->port->a.fire()) {
            auto& chnA = this->port->a;
            // Log("[%ld] [A fire] addr: %hx\n", *cycles, *chnA.address);
            chnA.valid = false;
            tlc_assert(pendingA.is_pending(), this, "No pending A but A fired!");
            pendingA.update(this);
            if (!pendingA.is_pending()) { // req A finished
                this->localBoard->query(this, pendingA.info->address)->update_status(this, S_A_WAITING_D, pendingA.info->alias);
            }
        }
    }

    void CAgent::fire_b() {
        if (this->port->b.fire()) {
            auto& chnB = this->port->b;
            // Log("[%ld] [B fire] addr: %hx\n", *cycles, *chnB.address);
            auto req_b = std::make_shared<BundleChannelB>();
            req_b->opcode   = chnB.opcode;
            req_b->address  = chnB.address;
            req_b->param    = chnB.param;
            req_b->size     = chnB.size;
            req_b->source   = chnB.source;
            req_b->alias    = (chnB.alias) >> 1;
            req_b->needdata = (chnB.alias) & 0x1;
            pendingB.init(req_b, 1);
            Log(this, Append("[Probe ", ProbeParamToString(chnB.param), "] ")
                .Hex().ShowBase().Append("source: ", uint64_t(chnB.source), ", addr: ", chnB.address, ", alias: ", (chnB.alias) >> 1).EndLine());
        }
    }

    void CAgent::fire_c() {
        if (this->port->c.fire()) {
            auto& chnC = this->port->c;
            bool releaseHasData = chnC.opcode == ReleaseData;
            bool needAck = chnC.opcode == ReleaseData || chnC.opcode == Release;
            bool probeAckDataToB = chnC.opcode == ProbeAckData && (chnC.param == TtoB || chnC.param == BtoB);
            tlc_assert(pendingC.is_pending(), this, "No pending C but C fired!");
            pendingC.update(this);
            if (!pendingC.is_pending()) { // req C finished
                chnC.valid = false;
                // Log("[%ld] [C fire] addr: %hx opcode: %hx\n", *cycles, *chnC.address, *chnC.opcode);
                auto info = this->localBoard->query(this, pendingC.info->address);
                auto exact_status = info->status[pendingC.info->alias];
                if (needAck) {
                    info->update_status(this, S_C_WAITING_D, pendingC.info->alias);
                } else {
                    if (exact_status == S_C_WAITING_D_INTR) {
                        info->update_status(this, S_C_WAITING_D, pendingC.info->alias);
                    } else if (exact_status == S_A_WAITING_D_INTR || exact_status == S_A_WAITING_D) {
                        info->update_status(this, S_A_WAITING_D, pendingC.info->alias);
                    } else {
                        if (probeAckDataToB) {
                            info->update_status(this, S_VALID, pendingC.info->alias);
                        } else {
                            info->update_status(this, S_INVALID, pendingC.info->alias);
                        }
                    }
                }
                if (releaseHasData) {
                    auto global_SBEntry = std::make_shared<Global_SBEntry>();
                    global_SBEntry->pending_data = pendingC.info->data;
                    if (this->globalBoard->get().count(pendingC.info->address) == 0) {
                        global_SBEntry->data = nullptr;
                    } else {
                        global_SBEntry->data = this->globalBoard->get()[pendingC.info->address]->data;
                    }
                    global_SBEntry->status = Global_SBEntry::SB_PENDING;
                    this->globalBoard->update(this, pendingC.info->address, global_SBEntry);
                }
                if (chnC.opcode == ProbeAckData) {
                    auto global_SBEntry = std::make_shared<Global_SBEntry>();
                    global_SBEntry->data = pendingC.info->data;
                    global_SBEntry->status = Global_SBEntry::SB_VALID;
                    this->globalBoard->update(this, pendingC.info->address, global_SBEntry);
                }
                if (chnC.opcode == ReleaseData || chnC.opcode == Release) {
                    info->update_pending_priviledge(this, shrinkGenPriv(this, pendingC.info->param), pendingC.info->alias);
                } else {
                    if (chnC.opcode == ProbeAck || chnC.opcode == ProbeAckData) {
                      info->update_priviledge(this, shrinkGenPriv(this, pendingC.info->param), pendingC.info->alias);
                    }
                    // Log("== free == fireC %d\n", *chnC.source);
                    this->probeIDpool.freeid(chnC.source);
                }

            }
        }
    }

    void CAgent::fire_d() {
        if (this->port->d.fire()) {
            auto& chnD = this->port->d;
            bool hasData = chnD.opcode == GrantData;
            bool grant = chnD.opcode == GrantData || chnD.opcode == Grant;
            auto addr = idMap->query(this, chnD.source)->address;
            auto alias = idMap->query(this, chnD.source)->alias;
            auto info = localBoard->query(this, addr);
            auto exact_status = info->status[alias];
            if (!(exact_status == S_C_WAITING_D || exact_status == S_A_WAITING_D || exact_status == S_C_WAITING_D_INTR || exact_status == S_A_WAITING_D_INTR || exact_status == S_INVALID)) {
              Log(this, Append("fire_d: status of localboard is ", exact_status).EndLine());
              Log(this, Hex().ShowBase().Append("addr: ", addr).EndLine());
              tlc_assert(false, this, Gravity::StringAppender("Status error! Not expected to received from channel D.").EndLine()
                    .Append("current status: ", StatusToString(exact_status)).EndLine()
                    .Append("description: ", StatusToDescription(exact_status)).EndLine()
                .ToString());
            }
            if (pendingD.is_pending()) { // following beats
                tlc_assert(chnD.opcode == pendingD.info->opcode, this, "Opcode mismatch among beats!");
                tlc_assert(chnD.param  == pendingD.info->param,  this, "Param mismatch among beats!");
                tlc_assert(chnD.source == pendingD.info->source, this, "Source mismatch among beats!");
                pendingD.update(this);
            } else { // new D resp
                auto resp_d = std::make_shared<BundleChannelD<RespField, EchoField, DATASIZE>>();
                resp_d->opcode  = chnD.opcode;
                resp_d->param   = chnD.param;
                resp_d->source  = chnD.source;
                resp_d->data    = grant ? make_shared_tldata<DATASIZE>() : nullptr;
                int nr_beat = (chnD.opcode == Grant || chnD.opcode == ReleaseAck) ? 0 : 1; // TODO: parameterize it
                pendingD.init(resp_d, nr_beat);
            }
            if (hasData) {
                int beat_num = pendingD.nr_beat - pendingD.beat_cnt;
                /*
                for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
                    pendingD.info->data[i] = chnD.data[i - BEATSIZE * beat_num];
                }
                */
                std::memcpy((uint8_t*)(pendingD.info->data->data) + BEATSIZE * beat_num, chnD.data->data, BEATSIZE);

#               ifdef CAGENT_DEBUG
                    Debug(this, Append("[CAgent] channel D receiving data: "));
                    DebugEx(data_dump_embedded<BEATSIZE>(chnD.data->data));
                    DebugEx(std::cout << std::endl);
#               endif
            }
            if (!pendingD.is_pending()) {
                switch (chnD.opcode) {
                    case GrantData: {
                        Log(this, Append("[GrantData] ")
                            .Hex().ShowBase().Append("source: ", uint64_t(chnD.source), ", addr: ", addr, ", alias: ", alias, ", data: "));
                        LogEx(data_dump_embedded<DATASIZE>(pendingD.info->data->data));
                        LogEx(std::cout << std::endl;);
                        
                        this->globalBoard->verify(this, addr, pendingD.info->data);
                        // info->update_dirty(*chnD.dirty, alias);
                        break;
                    }
                    case Grant: {
                        Log(this, Append("[Grant] ")
                            .Hex().ShowBase().Append("source: ", uint64_t(chnD.source), ", addr: ", addr, ", alias: ", alias).EndLine());
                        // Always set dirty in acquireperm txns
                        info->update_dirty(this, true, alias);
                        break;
                    }
                    case ReleaseAck: {
                        Log(this, Append("[ReleaseAck] ")
                            .Hex().ShowBase().Append("source: ", uint64_t(chnD.source), ", addr: ", addr, ", alias: ", alias).EndLine());
                        if (exact_status == S_C_WAITING_D) {
                            info->update_status(this, S_INVALID, alias);
                            info->update_dirty(this, 0, alias);
                        } else {
                            tlc_assert(exact_status == S_C_WAITING_D_INTR, this, 
                                Gravity::StringAppender("Status error! ReleaseAck not expected.").EndLine()
                                    .Append("current status: ", StatusToString(exact_status)).EndLine()
                                    .Append("description: ", StatusToDescription(exact_status)).EndLine()
                                .ToString());
                            info->update_status(this, S_SENDING_C, alias);
                        }
                        info->unpending_priviledge(this, alias);
                        this->globalBoard->unpending(this, addr);
                        break;
                    }
                    default:
                        tlc_assert(false, this, "Unknown opcode in channel D!");
                }

                // Send E
                if (grant) {
                    tlc_assert(exact_status != S_A_WAITING_D_INTR, this, "TODO: check this Ridiculous probe!");
                    auto req_e = std::make_shared<BundleChannelE>();
                    req_e->sink     = chnD.sink;
                    req_e->addr     = addr;
                    req_e->alias    = alias;
                    if (pendingE.is_pending()) {
                        tlc_assert(false, this, "E is pending!");
                    }
                    pendingE.init(req_e, 1);
                    info->update_status(this, S_SENDING_E, alias);
                    info->update_priviledge(this, capGenPriv(this, chnD.param), alias);
                }
                idMap->erase(this, chnD.source);
                // Log("== free == fireD %d\n", *chnD.source);
                this->idpool.freeid(chnD.source);
            }
        }
    }

    void CAgent::fire_e() {
        if (this->port->e.fire()) {
            auto& chnE = this->port->e;
            chnE.valid = false;
            tlc_assert(pendingE.is_pending(), this, "No pending E but E fired!");
            auto info = localBoard->query(this, pendingE.info->addr);
            info->update_status(this, S_VALID, pendingE.info->alias);
            pendingE.update(this);
        }
    }

    void CAgent::handle_channel() {
        // Constraint: fire_e > fire_d, otherwise concurrent D/E requests will disturb the pendingE
        fire_a();
        fire_b();
        fire_c();
        fire_e();
        fire_d();
    }

    void CAgent::update_signal() {
        this->port->d.ready = true; // TODO: do random here
        this->port->b.ready = !(pendingB.is_pending());

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
        idpool.update(this);
        probeIDpool.update(this);
    }

    bool CAgent::do_acquireBlock(paddr_t address, int param, int alias) {
        if (pendingA.is_pending() || pendingB.is_pending() || idpool.full())
            return false;
        if (localBoard->haskey(address)) { // check whether this transaction is legal
            auto entry = localBoard->query(this, address);
#           if CAGENT_NO_ALIAS_ACQUIRE == 1
            for (int i = 0; i < 4; i++) {
#           else
            int i = alias; {
#           endif
                auto privilege = entry->privilege[i];
                auto status = entry->status[i];
                if (status != S_VALID && status != S_INVALID) {
                    return false;
                }
                if (status == S_VALID) {
                    if (privilege == TIP) return false;
                    // if (privilege == BRANCH && param != BtoT) { param = BtoT; }
                    if (privilege == BRANCH && param != BtoT) return false;
                    if (privilege == INVALID && param == BtoT) return false;
                }
            }
        }
        auto req_a = std::make_shared<BundleChannelA<ReqField, EchoField, DATASIZE>>();
        req_a->opcode   = AcquireBlock;
        req_a->address  = address;
        req_a->param    = param;
        req_a->size     = ceil(log2((double)DATASIZE));
        req_a->mask     = (0xffffffffUL);
        req_a->source   = this->idpool.getid();
        req_a->alias    = alias;
        // Log("== id == acquire %d\n", *req_a->source);
        pendingA.init(req_a, 1);
        Log(this, Append("[AcquireBlock ", AcquireParamToString(param), "] ")
                .Hex().ShowBase().Append("source: ", uint64_t(req_a->source), ", addr: ", address, ", alias: ", alias).EndLine());

        return true;
    }

    bool CAgent::do_acquirePerm(paddr_t address, int param, int alias) {
        if (pendingA.is_pending() || pendingB.is_pending() || idpool.full())
            return false;
        if (localBoard->haskey(address)) {
            auto entry = localBoard->query(this, address);
#           if CAGENT_NO_ALIAS_ACQUIRE == 1
            for (int i = 0; i < 4; i++) {
#           else
            int i = alias; {
#           endif
                auto privilege = entry->privilege[i];
                auto status = entry->status[i];
                if (status != S_VALID && status != S_INVALID) {
                    return false;
                }
                if (status == S_VALID) {
                    if (privilege == TIP) return false;
                    if (privilege == BRANCH && param != BtoT) { param = BtoT; }
                    if (privilege == INVALID && param == BtoT) return false;
                }
            }
        }
        auto req_a = std::make_shared<BundleChannelA<ReqField, EchoField, DATASIZE>>();
        req_a->opcode   = AcquirePerm;
        req_a->address  = address;
        req_a->param    = param;
        req_a->size     = ceil(log2((double)DATASIZE));
        req_a->mask     = (0xffffffffUL);
        req_a->source   = this->idpool.getid();
        req_a->alias    = alias;
        // Log("== id == acquire %d\n", *req_a->source);
        pendingA.init(req_a, 1);
        Log(this, Append("[AcquirePerm ", AcquireParamToString(param) , "] ")
            .Hex().ShowBase().Append("source: ", uint64_t(req_a->source), ", addr: ", address, ", alias: ", alias).EndLine());
        return true;
    }

    bool CAgent::do_releaseData(paddr_t address, int param, shared_tldata_t<DATASIZE> data, int alias) {
        if (pendingC.is_pending() || pendingB.is_pending() || idpool.full() || !localBoard->haskey(address))
            return false;
        // TODO: checkout pendingA
        // TODO: checkout pendingB - give way?
        auto entry = localBoard->query(this, address);
        auto privilege = entry->privilege[alias];
        auto status = entry->status[alias];
        if (status != S_VALID) {
            return false;
        }
        if (privilege == INVALID) return false;
        if (privilege == BRANCH && param != BtoN) return false;
        if (privilege == TIP && param == BtoN) return false;

        auto req_c = std::make_shared<BundleChannelC<ReqField, EchoField, DATASIZE>>();
        req_c->opcode   = ReleaseData;
        req_c->address  = address;
        req_c->param    = param;
        req_c->size     = ceil(log2((double)DATASIZE));
        req_c->source   = this->idpool.getid();
        req_c->dirty    = 1;
        // Log("== id == release %d\n", *req_c->source);
        req_c->data     = data;
        req_c->alias    = alias;
        pendingC.init(req_c, DATASIZE / BEATSIZE);
        Log(this, Append("[ReleaseData ", ReleaseParamToString(param), "] ")
            .Hex().ShowBase().Append("source: ", uint64_t(req_c->source), ", addr: ", address, ", alias: ", alias, ", data: "));
        LogEx(data_dump_embedded<DATASIZE>(data->data));
        LogEx(std::cout << std::endl);
        return true;
    }

    bool CAgent::do_releaseDataAuto(paddr_t address, int alias) {
        if (pendingC.is_pending() || pendingB.is_pending() || idpool.full() || !localBoard->haskey(address))
            return false;
        // TODO: checkout pendingA
        // TODO: checkout pendingB - give way?
        auto entry = localBoard->query(this, address);
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
            tlc_assert(false, this, "Invalid priviledge detected!");
        }
        auto status = entry->status[alias];
        if (status != S_VALID) {
            return false;
        }

        auto req_c = std::make_shared<BundleChannelC<ReqField, EchoField, DATASIZE>>();
        req_c->opcode   = ReleaseData;
        req_c->address  = address;
        req_c->param    = param;
        req_c->size     = ceil(log2((double)DATASIZE));
        req_c->source   = this->idpool.getid();
        req_c->dirty    = 1;
        req_c->alias    = alias;
        if (param == BtoN) {
            if (globalBoard->haskey(address))   // Data all-zero when present in LocalBoard but not in GlobalBoard
                req_c->data = globalBoard->query(this, address)->data;
            else
            {
                req_c->data = make_shared_tldata<DATASIZE>();
                for (int i = 0; i < DATASIZE; i++) {
                    req_c->data->data[i] = 0;
                }
            }
        } else {
            tlc_assert(param == TtoN, this, "Wrong execution path!");
            req_c->data = make_shared_tldata<DATASIZE>();
            for (int i = 0; i < DATASIZE; i++) {
                req_c->data->data[i] = (uint8_t)CAGENT_RAND64(this, "CAgent");
            }
#           ifdef CAGENT_DEBUG
                Debug(this, Append("do_releaseDataAuto(): randomized data: "));
                DebugEx(data_dump_embedded<DATASIZE>(req_c->data->data));
                DebugEx(std::cout << std::endl);
#           endif
        }

        // Log("== id == release %d\n", *req_c->source);
        pendingC.init(req_c, DATASIZE / BEATSIZE);
        Log(this, Append("[ReleaseData ", ReleaseParamToString(param), "] ")
                .Hex().ShowBase().Append("source: ", uint64_t(req_c->source), ", addr: ", address, ", alias: ", alias, ", data: "));
        LogEx(data_dump_embedded<DATASIZE>(req_c->data->data));
        LogEx(std::cout << std::endl);

        return true;
    }

    void CAgent::timeout_check() {
        if (localBoard->get().empty()) {
            return;
        }
        for (auto it = this->localBoard->get().begin(); it != this->localBoard->get().end(); it++) {
            auto addr = it->first;
            auto value = it->second;
            for(int i = 0; i < 4; i++){
              if (value->status[i] != S_INVALID && value->status[i] != S_VALID) {
                if (*this->cycles - value->time_stamp > TIMEOUT_INTERVAL) {

                    std::cout << Gravity::StringAppender().ShowBase()
                        .Hex().Append("Address:     ", addr)
                        .Dec().Append("Now time:    ", *this->cycles)
                        .Dec().Append("Last stamp:  ", value->time_stamp)
                        .Dec().Append("Status[0]:   ", StatusToString(value->status[0]))
                    .ToString();

                    tlc_assert(false, this, "Transaction time out");
                }
              }
            }
        }
    }
}
