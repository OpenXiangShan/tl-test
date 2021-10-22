//
// Created by ljw on 10/21/21.
//

#ifndef TLC_TEST_PORT_H
#define TLC_TEST_PORT_H

#include <cstddef>
#include <cstdint>
#include <array>

namespace tl_agent {

    class Decoupled {
    public:
        uint8_t *valid;
        uint8_t *ready;

        bool fire() const{
            return *valid && *ready;
        }
    };

    template<class Usr, class Echo>
    class ChnA : public Decoupled {
    public:
        uint8_t *opcode;
        uint8_t *param;
        uint8_t *size;
        uint8_t *source;
        uint64_t *address;
        Usr *usr;
        Echo *echo;
        uint8_t *corrupt;
    };

    class ChnB : public Decoupled {
    public:
        uint8_t *opcode;
        uint8_t *param;
        uint8_t *size;
        uint8_t *source;
        uint64_t *address;
        uint8_t *corrupt;
    };

    template<class Usr, class Echo, std::size_t N>
    class ChnC : public Decoupled {
    public:
        uint8_t *opcode;
        uint8_t *param;
        uint8_t *size;
        uint8_t *source;
        uint64_t *address;
        Usr *usr;
        Echo *echo;
        std::array<uint8_t, N> *data;
        uint8_t *corrupt;
    };

    template<class Usr, class Echo, std::size_t N>
    class ChnD : public Decoupled {
    public:
        uint8_t *opcode;
        uint8_t *param;
        uint8_t *size;
        uint8_t *source;
        uint8_t *sink;
        uint8_t *denied;
        Usr *usr;
        Echo *echo;
        std::array<uint8_t, N> *data;
        uint8_t *corrupt;
    };

    class ChnE : public Decoupled {
    public:
        uint8_t *sink;
    };

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    class Port {
    public:
        ChnA<ReqField, EchoField> a;
        ChnB b;
        ChnC<ReqField, EchoField, N> c;
        ChnD<RespField, EchoField, N> d;
        ChnE e;
    };

}

#endif //TLC_TEST_PORT_H
