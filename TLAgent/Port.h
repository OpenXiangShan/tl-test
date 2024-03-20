//
// Created by ljw on 10/21/21.
//

#ifndef TLC_TEST_PORT_H
#define TLC_TEST_PORT_H

#include "../Utils/Common.h"
#include <cstddef>
#include <cstdint>
#include <array>

namespace tl_agent {

    class DecoupledIO {
    public:
        uint8_t *valid;
        uint8_t *ready;

        bool fire() const {
            return *valid && *ready;
        }
    };

    template<class Usr, class Echo, std::size_t N>
    class IOChannelA : public DecoupledIO {
    public:
        uint8_t     *opcode;
        uint8_t     *param;
        uint8_t     *size;
        uint8_t     *source;
        uint32_t    *mask;
        paddr_t     *address;
        uint8_t     *data;
        Usr         *usr;
        Echo        *echo;
        uint8_t     *corrupt;
        uint8_t     *alias;
    };

    class IOChannelB : public DecoupledIO {
    public:
        uint8_t     *opcode;
        uint8_t     *param;
        uint8_t     *size;
        uint8_t     *source;
        paddr_t     *address;
        uint8_t     *corrupt;
        uint8_t     *alias;
        uint8_t     *needdata;
    };

    template<class Usr, class Echo, std::size_t N>
    class IOChannelC : public DecoupledIO {
    public:
        uint8_t     *opcode;
        uint8_t     *param;
        uint8_t     *size;
        uint8_t     *source;
        paddr_t     *address;
        Usr         *usr;
        Echo        *echo;
        uint8_t     *dirty;
        uint8_t     *data;
        uint8_t     *corrupt;
        uint8_t     *alias;
    };

    template<class Usr, class Echo, std::size_t N>
    class IOChannelD : public DecoupledIO {
    public:
        uint8_t     *opcode;
        uint8_t     *param;
        uint8_t     *size;
        uint8_t     *source;
        uint8_t     *sink;
        uint8_t     *denied;
        Usr         *usr;
        Echo        *echo;
        uint8_t     *dirty;
        uint8_t     *data;
        uint8_t     *corrupt;
    };

    class IOChannelE : public DecoupledIO {
    public:
        uint8_t     *sink;
        paddr_t     *addr;  // used for index scoreboard
        uint8_t     *alias;
    };

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    class Port {
    public:
        IOChannelA<ReqField, EchoField, N>  a;
        IOChannelB                          b;
        IOChannelC<ReqField, EchoField, N>  c;
        IOChannelD<RespField, EchoField, N> d;
        IOChannelE                          e;
    };

}

#endif //TLC_TEST_PORT_H
