//
// Created by zcy on 2023/3/2.
//

#include "tl_ul_agent.h"

using namespace tl_agent;

namespace tl_ul_agent {

    inline paddr_t align_addr(paddr_t addr) { return addr & 0xFFFFFFC0; }

    void TLULAgent::fire_a() {
        if (this->port->a.fire()) {
            *pendingA.info->address = align_addr(*pendingA.info->address);
            auto &channel_a = this->port->a;
            *channel_a.valid = false;
            tlc_assert(pendingA.is_pending(), "No pending A but A fired!");
            pendingA.update();
            
            a_fire = true;
        }
    }

    void TLULAgent::fire_b() {}

    void TLULAgent::fire_c() {}

    void TLULAgent::fire_d() {
        if (this->port->d.fire()) {
            d_fire = true;
        }
    }

    void TLULAgent::fire_e() {}

    Resp TLULAgent::send_a(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE> >a) {
        *this->port->a.opcode = *a->opcode;
        *this->port->a.address = *a->address;
        *this->port->a.size = *a->size;
        *this->port->a.mask = *a->mask;
        *this->port->a.source = *a->source;
        *this->port->a.valid = true;
        return OK;
    }

    void TLULAgent::handle_b(std::shared_ptr<ChnB>b) {

    }

    Resp TLULAgent::send_c(std::shared_ptr<ChnC<ReqField, EchoField, DATASIZE> >c) {

    }

}

