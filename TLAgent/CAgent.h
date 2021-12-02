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
        int status;
        int privilege;
        int pending_privilege;
        C_SBEntry(int status, int privilege, uint64_t& time) {
            this->time_stamp = time;
            this->privilege = privilege;
            this->status = status;
        }
        void update_status(int status, uint64_t& time) {
            this->status = status;
            this->time_stamp = time;
        }
        void update_priviledge(int priv, uint64_t& time) {
            this->privilege = priv;
            this->time_stamp = time;
        }
        void update_pending_priviledge(int priv, uint64_t& time) {
            this->pending_privilege = priv;
            this->time_stamp = time;
        }
        void unpending_priviledge(uint64_t& time) {
            this->privilege = this->pending_privilege;
            this->pending_privilege = -1;
            this->time_stamp = time;
        }
    };

    class C_IDEntry {
    public:
        paddr_t address;
        C_IDEntry(paddr_t &addr) {
            this->address = addr;
        }
    };

    class CAgent : public BaseAgent {
    private:
        uint64_t *cycles;
        PendingTrans<ChnA<ReqField, EchoField, DATASIZE>> pendingA;
        PendingTrans<ChnB> pendingB;
        PendingTrans<ChnC<ReqField, EchoField, DATASIZE>> pendingC;
        PendingTrans<ChnD<RespField, EchoField, DATASIZE>> pendingD;
        PendingTrans<ChnE> pendingE;
        /* Here we need a scoreboard maintaining address->info
         * For convenience, an idMap(id->addr) is also maintained
         */
        ScoreBoard<paddr_t , C_SBEntry> *localBoard;
        ScoreBoard<int, C_IDEntry> *idMap;
        IDPool probeIDpool;
        void timeout_check();

    public:
        CAgent(GlobalBoard<paddr_t> * const gb, int id, uint64_t* cycles);
        ~CAgent() = default;
        Resp send_a(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> &a);
        void handle_b(std::shared_ptr<ChnB> &b);
        Resp send_c(std::shared_ptr<ChnC<ReqField, EchoField, DATASIZE>> &c);
        void handle_d();
        Resp send_e(std::shared_ptr<ChnE> &e);
        void fire_a();
        void fire_b();
        void fire_c();
        void fire_d();
        void fire_e();
        void handle_channel();
        void update_signal();

        bool do_acquireBlock(paddr_t address, int param);
        bool do_acquirePerm(paddr_t address, int param);
        bool do_releaseData(paddr_t address, int param, uint8_t data[]);
    };

}



#endif //TLC_TEST_CAGENT_H
