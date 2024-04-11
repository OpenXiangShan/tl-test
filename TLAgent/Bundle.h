//
// Created by ljw on 10/21/21.
//

#ifndef TLC_TEST_BUNDLE_H
#define TLC_TEST_BUNDLE_H

#include "../Utils/Common.h"
#include <cstddef>
#include <cstdint>
#include <array>
#include <memory>

namespace tl_agent {

    //
    class DecoupledBundle {
    public:
        uint8_t     valid;
        uint8_t     ready;

        bool fire() const {
            return valid && ready;
        }
    };

    template<class Usr, class Echo, std::size_t N>
    class BundleChannelA : public DecoupledBundle {
    public:
        uint8_t             opcode;
        uint8_t             param;
        uint8_t             size;
        uint8_t             source;
        uint32_t            mask;
        paddr_t             address;
        shared_tldata_t<N>  data;
        Usr                 usr;
        Echo                echo;
        uint8_t             corrupt;
        uint8_t             alias;
    };

    class BundleChannelB : public DecoupledBundle {
    public:
        uint8_t             opcode;
        uint8_t             param;
        uint8_t             size;
        uint8_t             source;
        paddr_t             address;
        uint8_t             corrupt;
        uint8_t             alias;
        uint8_t             needdata;
    };

    template<class Usr, class Echo, std::size_t N>
    class BundleChannelC : public DecoupledBundle {
    public:
        uint8_t             opcode;
        uint8_t             param;
        uint8_t             size;
        uint8_t             source;
        paddr_t             address;
        Usr                 usr;
        Echo                echo;
        uint8_t             dirty;
        shared_tldata_t<N>  data;
        uint8_t             corrupt;
        uint8_t             alias;
    };

    template<class Usr, class Echo, std::size_t N>
    class BundleChannelD : public DecoupledBundle {
    public:
        uint8_t             opcode;
        uint8_t             param;
        uint8_t             size;
        uint8_t             source;
        uint8_t             sink;
        uint8_t             denied;
        Usr                 usr;
        Echo                echo;
        uint8_t             dirty;
        shared_tldata_t<N>  data;
        uint8_t             corrupt;
    };

    class BundleChannelE : public DecoupledBundle {
    public:
        uint8_t             sink;
        paddr_t             addr;  // used for index scoreboard
        uint8_t             alias;
    };

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    class Bundle {
    public:
        BundleChannelA<ReqField, EchoField, N>  a;
        BundleChannelB                          b;
        BundleChannelC<ReqField, EchoField, N>  c;
        BundleChannelD<RespField, EchoField, N> d;
        BundleChannelE                          e;
    };
}

#endif //TLC_TEST_BUNDLE_H
