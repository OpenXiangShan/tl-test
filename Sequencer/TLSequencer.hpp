#pragma once
//
// Created by Kumonda221 on 04/04/24
//

#include <cstdint>
#ifndef TLC_TEST_SEQUENCER_H
#define TLC_TEST_SEQUENCER_H

#include "../Utils/ScoreBoard.h"
#include "../Utils/Common.h"
#include "../TLAgent/ULAgent.h"
#include "../TLAgent/CAgent.h"
#include "../Fuzzer/Fuzzer.h"

/*
* Procedure sequence of a TileLink cycle:
*   Tick() -> FireChannel*() -> Tock() -> UpdateChannel*()
*/
class TLSequencer {
public:
    using BaseAgent         = tl_agent::BaseAgent;
    using ULAgent           = tl_agent::ULAgent;
    using CAgent            = tl_agent::CAgent;

    using ReqField          = tl_agent::ReqField;
    using RespField         = tl_agent::RespField;
    using EchoField         = tl_agent::EchoField;

    using BundleChannelA    = tl_agent::BundleChannelA<ReqField, EchoField, DATASIZE>;
    using BundleChannelB    = tl_agent::BundleChannelB;
    using BundleChannelC    = tl_agent::BundleChannelC<ReqField, EchoField, DATASIZE>;
    using BundleChannelD    = tl_agent::BundleChannelD<RespField, EchoField, DATASIZE>;
    using BundleChannelE    = tl_agent::BundleChannelE;
    using Bundle            = tl_agent::Bundle<ReqField, RespField, EchoField, DATASIZE>;

    using IOChannelA        = tl_agent::BundleChannelA<ReqField, EchoField, BEATSIZE>;
    using IOChannelB        = tl_agent::BundleChannelB;
    using IOChannelC        = tl_agent::BundleChannelC<ReqField, EchoField, BEATSIZE>;
    using IOChannelD        = tl_agent::BundleChannelD<RespField, EchoField, BEATSIZE>;
    using IOChannelE        = tl_agent::BundleChannelE;
    using IOPort            = tl_agent::Bundle<ReqField, RespField, EchoField, BEATSIZE>;

    static constexpr int NR_AGENTS  = NR_CAGENTS + NR_ULAGENTS;

private:
    GlobalBoard<paddr_t>*   globalBoard;

    BaseAgent**             const agents;
    Fuzzer**                const fuzzers;

    IOPort**                const io;

    uint64_t                cycles;

    uint64_t                seed;

public:
    TLSequencer() noexcept;
    ~TLSequencer() noexcept;

    void        Initialize() noexcept;
    void        Finalize() noexcept;
    void        Tick(uint64_t cycles) noexcept;
    void        Tock() noexcept;

    IOPort*     IO() noexcept;
    IOPort&     IO(int deviceId) noexcept;

    void        FireChannelA() noexcept;
    void        FireChannelB() noexcept;
    void        FireChannelC() noexcept;
    void        FireChannelD() noexcept;
    void        FireChannelE() noexcept;

    void        UpdateChannelA() noexcept;
    void        UpdateChannelB() noexcept;
    void        UpdateChannelC() noexcept;
    void        UpdateChannelD() noexcept;
    void        UpdateChannelE() noexcept;
};


#endif //TLC_TEST_SEQUENCER_H
