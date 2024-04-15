//
// Created by wkf on 2021/11/2.
//

#ifndef TLC_TEST_CAGENT_H
#define TLC_TEST_CAGENT_H

#include "BaseAgent.h"
#include "../Utils/Common.h"
#include "../Utils/ScoreBoard.h"
#include "Bundle.h"


#if AGENT_DEBUG == 1
#   define CAGENT_DEBUG
#endif


namespace tl_agent {

    class C_SBEntry {
    public:
        uint64_t time_stamp;
        int status[4];
        int privilege[4];
        int pending_privilege[4];
        int dirty[4];

        C_SBEntry(const int status[], const int privilege[], uint64_t& time) {
            this->time_stamp = time;
            for(int i = 0; i<4; i++){
              this->privilege[i] = privilege[i];
              this->status[i] = status[i];
            }
        }
        void update_status(int status, uint64_t& time, int alias) {
            this->status[alias] = status;
            this->time_stamp = time;
        }
        void update_priviledge(int priv, uint64_t& time, int alias) {
            this->privilege[alias] = priv;
            this->time_stamp = time;
        }
        void update_pending_priviledge(int priv, uint64_t& time, int alias) {
            this->pending_privilege[alias] = priv;
            this->time_stamp = time;
        }
        void unpending_priviledge(uint64_t& time, int alias) {
            this->privilege[alias] = this->pending_privilege[alias];
            this->pending_privilege[alias] = -1;
            this->time_stamp = time;
        }
        void update_dirty(int dirty, int alias) {
            this->dirty[alias] = dirty;
        }
    };

    class C_IDEntry {
    public:
        paddr_t address;
        int alias;
        C_IDEntry(paddr_t &addr, uint8_t &alias) {
            this->address = addr;
            this->alias = alias;
        }
    };


#   ifdef false
#       define CAGENT_RAND64(agent, source) ( \
            agent->aux_rand_value = agent->rand64(), \
            std::cout << Gravity::StringAppender("[tl-test-passive-DEBUG] ") \
                .Append("rand64() called: from ", source, ", counter = ", agent->aux_rand_counter++) \
                .Hex().ShowBase() \
                .Append(", seed = ").Append(agent->sysSeed()) \
                .Append(", value = ").Append(agent->aux_rand_value).EndLine() \
                .ToString(), \
            agent->aux_rand_value)
#   endif

#   ifndef CAGENT_RAND64
#       define CAGENT_RAND64(agent, source) (agent->rand64())
#   endif

    class CAgent : public BaseAgent {
    private:
        uint64_t *cycles;
        PendingTrans<BundleChannelA<ReqField, EchoField, DATASIZE>> pendingA;
        PendingTrans<BundleChannelB> pendingB;
        PendingTrans<BundleChannelC<ReqField, EchoField, DATASIZE>> pendingC;
        PendingTrans<BundleChannelD<RespField, EchoField, DATASIZE>> pendingD;
        PendingTrans<BundleChannelE> pendingE;
        /* Here we need a scoreboard called localBoard maintaining address->info
         * For convenience, an idMap(id->addr) is also maintained
         */
        ScoreBoard<paddr_t , C_SBEntry> *localBoard;
        ScoreBoard<int, C_IDEntry> *idMap;
        IDPool probeIDpool;
        void timeout_check() override;

    public:
        CAgent(GlobalBoard<paddr_t> * const gb, int id, unsigned int seed, uint64_t* cycles) noexcept;
        virtual ~CAgent() noexcept;

        uint64_t    cycle() const noexcept override;

        Resp send_a     (std::shared_ptr<BundleChannelA<ReqField, EchoField, DATASIZE>>&    a) override;
        void handle_b   (std::shared_ptr<BundleChannelB>&                                   b) override;
        Resp send_c     (std::shared_ptr<BundleChannelC<ReqField, EchoField, DATASIZE>>&    c) override;
        Resp send_e     (std::shared_ptr<BundleChannelE>&                                   e);
        void fire_a() override;
        void fire_b() override;
        void fire_c() override;
        void fire_d() override;
        void fire_e() override;
        void handle_channel() override;
        void update_signal() override;

        bool do_acquireBlock(paddr_t address, int param, int alias);
        bool do_acquirePerm(paddr_t address, int param, int alias);
        bool do_releaseData(paddr_t address, int param, shared_tldata_t<DATASIZE> data, int alias);
        bool do_releaseDataAuto(paddr_t address, int alias);
    };

}



#endif //TLC_TEST_CAGENT_H
