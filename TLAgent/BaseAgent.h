//
// Created by ljw on 10/21/21.
//

#ifndef TLC_TEST_BASEAGENT_H
#define TLC_TEST_BASEAGENT_H

#include "Port.h"

namespace tl_agent {

    enum Resp {OK, FAIL};

    template<class Usr, class Echo, std::size_t N>
    class BaseAgent {
    private:
        Port<Usr, Echo, N> *port;

    public:
        virtual Resp send_a(ChnA<Usr> &a) = 0;
        virtual void handle_b(ChnB &b) = 0;
        virtual Resp send_c(ChnC<Usr, N> &c) = 0;
        virtual void handle_d(ChnD<Echo, N> &d) = 0;
        virtual void fire_a() = 0;
        virtual void fire_b() = 0;
        virtual void fire_c() = 0;
        virtual void fire_e() = 0;
        virtual ~BaseAgent() = 0;

        void connect(Port<Usr, Echo, N> *p){ this->port = p; }
    };

}

#endif //TLC_TEST_BASEAGENT_H
