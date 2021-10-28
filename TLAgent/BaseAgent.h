//
// Created by ljw on 10/21/21.
//

#ifndef TLC_TEST_BASEAGENT_H
#define TLC_TEST_BASEAGENT_H

#include <set>
#include "Port.h"
#include "../Utils/Common.h"
#include "../Utils/ScoreBoard.h"

namespace tl_agent {

    enum Resp {OK, FAIL};

    enum {
        S_INVALID = 0,
        S_VALID,
        S_SENDING_A,  // ready to send A request actively
        S_REACTING_B, // ready to react B request actively
        S_SENDING_C,  // ready to send C request actively
        S_WAITING_D,  // wait for D response
        S_SENDING_E,  // ready to send E request actively
    };

    class ReqField {
    public:
        uint8_t value;
    };

    class RespField {
    public:
        uint8_t value;
    };

    class EchoField {
    public:
        uint8_t value;
    };

    template<typename T>
    class PendingTrans {
    public:
        int beat_cnt;
        int nr_beat;
        std::shared_ptr<T> info;

        PendingTrans() {
            nr_beat = 0;
            beat_cnt = 0;
        }
        ~PendingTrans() = default;

        bool is_multiBeat() { return (this->nr_beat != 1); };
        bool is_pending() { return (beat_cnt != 0); }
        void init(std::shared_ptr<T> &info, int nr_beat) {
            this->info = info;
            this->nr_beat = nr_beat;
            beat_cnt = nr_beat;
        }
        void update() {
            beat_cnt--;
            tlc_assert(beat_cnt >= 0, "More beats received than expected!");
        }
    };

    class IDPool {
    private:
        std::set<int> *idle_ids;
        std::set<int> *used_ids;
    public:
        IDPool() {
            idle_ids = new std::set<int>();
            used_ids = new std::set<int>();
            for (int i = 0; i < NR_SOURCEID; i++) {
                idle_ids->insert(i);
            }
            used_ids->clear();
        }
        ~IDPool() {
            delete idle_ids;
            delete used_ids;
        }
        int getid() {
            if (idle_ids->size() == 0)
                return -1;
            int ret = *idle_ids->begin();
            used_ids->insert(ret);
            idle_ids->erase(ret);
            return ret;
        }
        void freeid(int id) {
            tlc_assert(used_ids->count(id) > 0, "Try to free unused SourceID!");
            used_ids->erase(id);
            idle_ids->insert(id);
        }
    };

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    class BaseAgent {
    protected:
        Port<ReqField, RespField, EchoField, BEATSIZE> *port;
        ScoreBoard<uint64_t, std::array<uint8_t, N>> *globalBoard;
        IDPool idpool;
        virtual void timeout_check() = 0;

    public:
        virtual Resp send_a(std::shared_ptr<ChnA<ReqField, EchoField, N>> &a) = 0;
        virtual void handle_b() = 0;
        virtual Resp send_c(std::shared_ptr<ChnA<ReqField, EchoField, N>> &c) = 0;
        virtual void handle_d() = 0;
        virtual void fire_a() = 0;
        virtual void fire_b() = 0;
        virtual void fire_c() = 0;
        virtual void fire_d() = 0;
        virtual void fire_e() = 0;
        virtual void update() = 0;
        BaseAgent(): idpool() {};
        virtual ~BaseAgent() = default;

        void connect(Port<ReqField, RespField, EchoField, BEATSIZE> *p){ this->port = p; }
    };

}

#endif //TLC_TEST_BASEAGENT_H
