//
// Created by zcy on 2023/2/27.
//

#include "tl_c_agent.h"


namespace tl_c_agent {

    void TLCAgent::fire_a() {
        if( this->port->a.fire()) {
            auto &channel_a = this->port->a;
            *channel_a.valid = false;

            tlc_assert(this->pendingA.is_pending(), "No pending A but A fired!");
            // update pending request
            pendingA.update();
            a_fire = true;
        }
    }

    void TLCAgent::fire_b() { // receive probe
        if (this->port->b.fire()) {
            auto &channel_b = this->port->b;
            // Log("[%ld] [B fire] addr: %lx\n", *cycles, *chnB.address);

            // allocate new pending request
            std::shared_ptr<ChnB>req_b(new ChnB());
            req_b->opcode.reset(new uint8_t(*channel_b.opcode));
            req_b->address.reset(new paddr_t(*channel_b.address));
            req_b->param.reset(new uint8_t(*channel_b.param));
            req_b->size.reset(new uint8_t(*channel_b.size));
            req_b->source.reset(new uint32_t(*channel_b.source));
            req_b->alias.reset(new uint8_t(*channel_b.alias));
            req_b->needdata.reset(new uint8_t(*channel_b.needdata));
            pendingB.init(req_b, 1);
            // TODO: rewrite this
            // std::string param_str = *channel_b.param==toN? "toN":(*channel_b.param==toB? "toB":(*channel_b.param==toT? "toT":"Unknown"));
            // Log("[%ld] [B] [Probe %s] addr: %lx alias: %d\n", *cycles, param_str.c_str(), *channel_b.address,
            //     (*channel_b.alias) >> 1); 
            b_fire = true;
        }
    }

    void TLCAgent::fire_c() {
        if (this->port->c.fire()) {
            tlc_assert(this->pendingC.is_pending(), "No pending C but C fired!");
            this->pendingC.update();

            c_fire = true;
        }
    }

    void TLCAgent::fire_d() {
        if(this->port->d.fire()) {
            auto &channel_d = this->port->d;
            using namespace tl_agent;
            bool hasData = *channel_d.opcode == GrantData;
            bool grant = *channel_d.opcode == GrantData || *channel_d.opcode == Grant;
            if (pendingD.is_pending()) { // following beats
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
                resp_d->data.reset(grant ? new uint8_t[DATASIZE] : nullptr);
                int nr_beat = (*channel_d.opcode == Grant || *channel_d.opcode == ReleaseAck)
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

            // if(grant) {
            //     std::shared_ptr<ChnE>req_e(new ChnE());
            //     req_e->sink.reset(new uint32_t(*channel_d.sink));
            //     req_e->addr.reset(new paddr_t(addr));
            //     req_e->alias.reset(new uint8_t(alias));
            //     if (pendingE.is_pending()) {
            //         Log("New E request when E is pending!\n");
            //         tlc_assert(false, "E is pending!");
            //     }
            //     pendingE.init(req_e, 1);
            // }

            this->d_fire = true;
        }
    }

    void TLCAgent::fire_e() {
        if (this->port->e.fire()) {
            auto &channel_e = this->port->e;
            *channel_e.valid = false;
            tlc_assert(this->pendingE.is_pending(), "No pending A but E fired!");
            this->pendingE.update();

            this->e_fire = true;
        }
    }

    Resp TLCAgent::send_a(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> a) {
        using namespace tl_agent;
        *this->port->a.opcode = *a->opcode;
        *this->port->a.address = *a->address;
        *this->port->a.size = *a->size;
        *this->port->a.param = *a->param;
        *this->port->a.mask = *a->mask;
        *this->port->a.source = *a->source;
        *this->port->a.alias = *a->alias;
        *this->port->a.valid = true;
        *this->port->a.preferCache = *a->preferCache;
        return OK;
    }

    void TLCAgent::handle_b (std::shared_ptr<ChnC<ReqField, EchoField, DATASIZE> >req_c) {
        // if (*req_c->opcode == ProbeAckData) {
        //     this->pendingC.init(req_c, DATASIZE / BEATSIZE);
        // } else {
        //     this->pendingC.init(req_c, 1);
        // }
        // this->pendingB.update();
    }

    Resp TLCAgent::send_c(std::shared_ptr<ChnC<ReqField, EchoField, DATASIZE>> c) {
        using namespace tl_agent;
        *this->port->c.opcode = *c->opcode;
        *this->port->c.param = *c->param;
        *this->port->c.address = *c->address;
        *this->port->c.size = *c->size;
        *this->port->c.source = *c->source;
        *this->port->c.dirty = *c->dirty;
        *this->port->c.valid = true;
        // data field is drived in higher level
        return OK;
    }

    Resp TLCAgent::send_e(std::shared_ptr<ChnE>e) {
        using namespace tl_agent;
        *this->port->e.sink = *e->sink;
        *this->port->e.valid = true;
        return OK;
    }
}

