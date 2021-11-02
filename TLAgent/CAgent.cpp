//
// Created by wkf on 2021/11/2.
//

#include "CAgent.h"

namespace tl_agent {
    CAgent::CAgent(GlobalBoard<paddr_t> *const gb, uint64_t *cycles):
        BaseAgent(), pendingA(), pendingD() //TODO
    {
        this->globalBoard = gb;
        this->cycles = cycles;
    }

    Resp CAgent::send_a(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> &a) {

    }

    void CAgent::handle_b() {

    }

    Resp CAgent::send_c(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> &c) {

    }

    void CAgent::handle_d() {

    }

    void CAgent::fire_a() {

    }

    void CAgent::fire_b() {

    }

    void CAgent::fire_c() {

    }

    void CAgent::fire_d() {

    }

    void CAgent::fire_e() {

    }

    void CAgent::handle_channel() {

    }

    void CAgent::update_signal() {

    }

    void CAgent::timeout_check() {

    }
}