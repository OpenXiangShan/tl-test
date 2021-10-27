//
// Created by ljw on 10/21/21.
//

#ifndef TLC_TEST_BASEAGENT_H
#define TLC_TEST_BASEAGENT_H

#include "Port.h"
#include "assert.h"
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
        T* info;

        PendingTrans() {
            nr_beat = 0;
            beat_cnt = 0;
        }
        ~PendingTrans() = default;

        bool is_multiBeat() { return (this->nr_beat != 1); };
        bool is_pending() { return (beat_cnt != 0); }
        void init(T* info, int nr_beat) {
            this->info = info;
            this->nr_beat = nr_beat;
            beat_cnt = nr_beat;
        }
        void update() {
            beat_cnt--;
            assert(beat_cnt >= 0);
            if (beat_cnt == 0) {
                delete info;
            }
        }
    };

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    class BaseAgent {
    protected:
        Port<ReqField, RespField, EchoField, N> *port;
        ScoreBoard<uint64_t, std::array<uint8_t, N>> *globalBoard;

    public:
        virtual Resp send_a(ChnA<ReqField, EchoField, N> &a) = 0;
        virtual void handle_b() = 0;
        virtual Resp send_c(ChnC<ReqField, EchoField, N> &c) = 0;
        virtual void handle_d() = 0;
        virtual void fire_a() = 0;
        virtual void fire_b() = 0;
        virtual void fire_c() = 0;
        virtual void fire_d() = 0;
        virtual void fire_e() = 0;
        virtual void update() = 0;
        virtual ~BaseAgent() = default;

        void connect(Port<ReqField, RespField, EchoField, N> *p){ this->port = p; }
    };

}

#endif //TLC_TEST_BASEAGENT_H
