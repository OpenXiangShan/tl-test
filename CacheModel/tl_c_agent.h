//
// Created by zcy on 2023/2/27.
//
#ifndef TL_C_AGENT_H
#define TL_C_AGENT_H

#include "tl_base_agent.h"

using namespace tl_agent;

namespace tl_c_agent {

class TLCAgent : public tl_base_agent::TLBaseAgent {
    public:
        // std::shared_ptr<Port<ReqField, RespField, EchoField, BEATSIZE>> port;

        // pending transactions
        PendingTrans<ChnA<ReqField, EchoField, DATASIZE>> pendingA;
        PendingTrans<ChnB> pendingB;
        PendingTrans<ChnC<ReqField, EchoField, DATASIZE>> pendingC;
        PendingTrans<ChnD<RespField, EchoField, DATASIZE>> pendingD;
        PendingTrans<ChnE> pendingE;

        TLCAgent(uint64_t *cycles, int id, int core_id, int bus_type): 
            TLBaseAgent(cycles, id, core_id, bus_type), 
            pendingA(), pendingB(), pendingC(), pendingD(), pendingE() {};
        ~TLCAgent() {};

        void fire_a();
        void fire_b();
        void fire_c();
        void fire_d();
        void fire_e();

        // transaction output process functions
        // we can only issue three type of transcations for TileLink bus in L1
        Resp send_a(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> a);
        void handle_b (std::shared_ptr<ChnC<ReqField, EchoField, DATASIZE> >req_c);
        Resp send_c(std::shared_ptr<ChnC<ReqField, EchoField, DATASIZE>> c);
        Resp send_e(std::shared_ptr<ChnE>e);
};

}

#endif // TL_C_AGENT_H