//
// Created by wkf on 2021/11/2.
//

#ifndef TLC_TEST_CAGENT_H
#define TLC_TEST_CAGENT_H

#include "BaseAgent.h"
#include "../Utils/Common.h"
#include "../Utils/ScoreBoard.h"
#include "Bundle.h"
#include "TLEnum.h"


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

        C_SBEntry(const TLLocalContext* ctx, const int status[], const int privilege[]) {
            this->time_stamp = ctx->cycle();
            for(int i = 0; i<4; i++){
              this->privilege[i] = privilege[i];
              this->status[i] = status[i];
            }
        }

        void update_status(const TLLocalContext* ctx, int status, int alias) {
            this->status[alias] = status;
            this->time_stamp = ctx->cycle();

#           if SB_DEBUG == 1
                Debug(ctx, Append("TL-C local scoreboard update (status): ")
                    .ShowBase()
                    .Dec().Append("[alias = ", alias, "]")
                    .Hex().Append(" status = ", StatusToString(status))
                    .EndLine());
#           endif
        }

        void update_priviledge(const TLLocalContext* ctx, int priv, int alias) {
            this->privilege[alias] = priv;
            this->time_stamp = ctx->cycle();

#           if SB_DEBUG == 1
                Debug(ctx, Append("TL-C local scoreboard update (privilege): ")
                    .ShowBase()
                    .Dec().Append("[alias = ", alias, "]")
                    .Hex().Append(" privilege = ", PrivilegeToString(priv))
                    .EndLine());
#           endif
        }

        void update_pending_priviledge(const TLLocalContext* ctx, int priv, int alias) {
            this->pending_privilege[alias] = priv;
            this->time_stamp = ctx->cycle();

#           if SB_DEBUG == 1
                Debug(ctx, Append("TL-C local scoreboard update (pending_privilege): ")
                    .ShowBase()
                    .Dec().Append("[alias = ", alias, "] ")
                    .Hex().Append(" pending_privilege = ", PrivilegeToString(priv))
                    .EndLine());
#           endif
        }

        void unpending_priviledge(const TLLocalContext* ctx, int alias) {
            this->privilege[alias] = this->pending_privilege[alias];
            this->pending_privilege[alias] = -1;
            this->time_stamp = ctx->cycle();

#           if SB_DEBUG == 1
                Debug(ctx, Append("TL-C local scoreboard update (unpending_privilege): ")
                    .ShowBase()
                    .Dec().Append("[alias = ", alias, "]")
                    .Hex().Append(" pending_privilege = ", PrivilegeToString(this->pending_privilege[alias]))
                    .EndLine());
#           endif
        }

        void update_dirty(const TLLocalContext* ctx, int dirty, int alias) {
            this->dirty[alias] = dirty;

#           if SB_DEBUG == 1
                Debug(ctx, Append("TL-C local scoreboard update (dirty): ")
                    .ShowBase()
                    .Dec().Append("[alias = ", alias, "]")
                    .Hex().Append(" dirty = ", PrivilegeToString(dirty))
                    .EndLine());
#           endif
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

    class C_AcquirePermEntry {
    public:
    };


    template<typename Tk>
    struct ScoreBoardUpdateCallbackCSBEntry : public ScoreBoardUpdateCallback<Tk, tl_agent::C_SBEntry>
    { 
        void update(const TLLocalContext* ctx, const Tk& key, std::shared_ptr<tl_agent::C_SBEntry>& data)
        {
#           if SB_DEBUG == 1
                Gravity::StringAppender strapp;

                strapp.Append("TL-C local scoreboard update: ")
                    .ShowBase()
                    .Hex().Append("key = ", uint64_t(key))
                //  .Dec().Append(", present = ", mapping.count(key))
                    .ToString();

                strapp.Append(", type = C_SBEntry");

                strapp.Append(", timestamp = ", data->time_stamp);

                strapp.EndLine();

                Debug(ctx, Append(strapp.ToString()));

                for (int i = 0; i < 4; i++)
                {
                    std::cout << Gravity::StringAppender("[", i, "] ")
                        .ShowBase()
                        .Append("status = ", tl_agent::StatusToString(data->status[i]))
                        .Append(", privilege = ", PrivilegeToString(data->privilege[i]))
                        .Append(", pending_privilege = ", PrivilegeToString(data->pending_privilege[i]))
                        .Append(", dirty = ", data->dirty[i])
                        .EndLine()
                        .ToString();
                }
#           endif 
        }
    };

    template<typename Tk>
    struct ScoreBoardUpdateCallbackCIDEntry : public ScoreBoardUpdateCallback<Tk, tl_agent::C_IDEntry>
    {
        void update(const TLLocalContext* ctx, const Tk& key, std::shared_ptr<tl_agent::C_IDEntry>& data)
        {
#           if SB_DEBUG == 1
                Debug(ctx, Append("TL-C local scoreboard update: ")
                    .ShowBase()
                    .Hex().Append("key = ", uint64_t(key))
                    .Dec().Append(", type = C_IDEntry")
                    .Hex().Append(", address = ", data->address)
                    .Hex().Append(", alias = ", data->alias)
                    .EndLine());
#           endif
        }
    };


#   ifdef false
#       define CAGENT_RAND64(agent, source) ( \
            agent->aux_rand_value = agent->rand64(), \
            Debug(this,  \
                Append("rand64() called: from ", source, ", counter = ", agent->aux_rand_counter++) \
                .Hex().ShowBase() \
                .Append(", seed = ").Append(agent->sysSeed()) \
                .Append(", value = ").Append(agent->aux_rand_value).EndLine() \
            ), \
            agent->aux_rand_value)
#   endif

#   ifndef CAGENT_RAND64
#       define CAGENT_RAND64(agent, source) (agent->rand64())
#   endif


    class CAgent : public BaseAgent {
    public:
        using LocalScoreBoard       = ScoreBoard<paddr_t, C_SBEntry, ScoreBoardUpdateCallbackCSBEntry<paddr_t>>;
        using IDMapScoreBoard       = ScoreBoard<paddr_t, C_IDEntry, ScoreBoardUpdateCallbackCIDEntry<paddr_t>>;
        using AcquirePermScoreBoard = ScoreBoard<paddr_t, C_AcquirePermEntry>;

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
        LocalScoreBoard*        localBoard;
        IDMapScoreBoard*        idMap;
        /*
        * *NOTICE:
        *   L1D infers that all following Release must obtain dirty data (must be ReleaseData)
        *   after the AcquirePerm with the same address.
        *   Besides, this is not straight-forward for inclusive system simulation, so the
        *   AcquirePermScoreBoard must be maintained.
        */
        AcquirePermScoreBoard*  acquirePermBoard;
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
        bool do_releaseDataAuto(paddr_t address, int alias, bool dirty, bool forced);
    };

}


#endif //TLC_TEST_CAGENT_H
