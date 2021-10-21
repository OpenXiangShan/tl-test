//
// Created by ljw on 10/21/21.
//

#ifndef TLC_TEST_PORT_H
#define TLC_TEST_PORT_H

#include "cstddef"

namespace tl_agent {

    template<class Usr>
    class ChnA {};

    class ChnB {};

    template<class Usr, std::size_t N>
    class ChnC {};

    template<class Echo, std::size_t N>
    class ChnD {};

    class ChnE {};

    template<class Usr, class Echo, std::size_t N>
    class Port {
    public:
        ChnA<Usr> a;
        ChnB b;
        ChnC<Usr, N> c;
        ChnD<Echo, N> d;
        ChnE e;
    };

}

#endif //TLC_TEST_PORT_H
