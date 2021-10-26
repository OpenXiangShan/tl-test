//
// Created by ljw on 10/21/21.
//

#ifndef TLC_TEST_BASEAGENT_H
#define TLC_TEST_BASEAGENT_H

#include "Port.h"
#include "../Utils/ScoreBoard.h"

namespace tl_agent {

    enum Resp {OK, FAIL};

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

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    class BaseAgent {
    protected:
        Port<ReqField, RespField, EchoField, N> *port;
        ScoreBoard<std::array<uint8_t, N>> *globalBoard;

    public:
        virtual Resp send_a(ChnA<ReqField, EchoField> &a) = 0;
        virtual void handle_b() = 0;
        virtual Resp send_c(ChnC<ReqField, EchoField, N> &c) = 0;
        virtual void handle_d() = 0;
        virtual void fire_a() = 0;
        virtual void fire_b() = 0;
        virtual void fire_c() = 0;
        virtual void fire_e() = 0;
        virtual ~BaseAgent() = default;

        void connect(Port<ReqField, RespField, EchoField, N> *p){ this->port = p; }
    };

}

#endif //TLC_TEST_BASEAGENT_H
