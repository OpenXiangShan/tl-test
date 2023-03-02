//
// Created by zcy on 2023/2/27.
//
#ifndef TL_UL_AGENT_H
#define TL_UL_AGENT_H

#include "tl_base_agent.h"

using namespace tl_agent;

namespace tl_ul_agent {


class TLULAgent : public tl_base_agent::TLBaseAgent {
    public:
        PendingTrans<ChnA<ReqField, EchoField, DATASIZE>> pendingA;
        PendingTrans<ChnD<RespField, EchoField, DATASIZE>> pendingD;

        TLULAgent(uint64_t *cycles, int id, int core_id, int bus_type): 
            TLBaseAgent(cycles, id, core_id, bus_type), 
            pendingA(), pendingD() {};
        ~TLULAgent() {};

        // transaction input process functions
        void fire_a();
        void fire_b();
        void fire_c();
        void fire_d();
        void fire_e();

        Resp send_a(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE> >a);
        void handle_b(std::shared_ptr<ChnB>b);
        Resp send_c(std::shared_ptr<ChnC<ReqField, EchoField, DATASIZE> >c);
};

}


#endif // TL_UL_AGENT_H