//
// Created by wkf on 2021/11/2.
//

#ifndef TLC_TEST_CAGENT_H
#define TLC_TEST_CAGENT_H

#include "BaseAgent.h"
#include "../Utils/Common.h"
#include "../Utils/ScoreBoard.h"

namespace tl_agent {

    class C_SBEntry {
    public:
        uint64_t time_stamp;
    };

    class CAgent : public BaseAgent {
    private:
        uint64_t *cycles;
        PendingTrans<ChnA<ReqField, EchoField, DATASIZE>> pendingA;
        PendingTrans<ChnD<RespField, EchoField, DATASIZE>> pendingD;
        /* Here we need a scoreboard maintaining address-> info */
        ScoreBoard<paddr_t , C_SBEntry> *localBoard;
        void timeout_check();

    public:
        CAgent(GlobalBoard<paddr_t> * const gb, uint64_t* cycles);
        ~CAgent() = default;
        Resp send_a(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> &a);
        void handle_b();
        Resp send_c(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> &c);
        void handle_d();
        void fire_a();
        void fire_b();
        void fire_c();
        void fire_d();
        void fire_e();
        void handle_channel();
        void update_signal();
    };

}



#endif //TLC_TEST_CAGENT_H
