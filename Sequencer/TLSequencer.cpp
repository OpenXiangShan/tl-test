//
//
//

#include "TLSequencer.hpp"
#include <cstddef>


TLSequencer::TLSequencer() noexcept
    : globalBoard   (nullptr)
    , config        ()
    , initialized   (false)
    , agents        (nullptr)
    , fuzzers       (nullptr)
    , io            (nullptr)
    , cycles        (0)
{ }

TLSequencer::~TLSequencer() noexcept
{
    delete[] fuzzers;
    delete[] agents;
    delete[] io;
}

size_t TLSequencer::GetCAgentCount() const noexcept
{
    return config.coreCount * config.masterCountPerCoreTLC;
}

size_t TLSequencer::GetULAgentCount() const noexcept
{
    return config.coreCount * config.masterCountPerCoreTLUL;
}

size_t TLSequencer::GetAgentCount() const noexcept
{
    return GetCAgentCount() + GetULAgentCount();
}

void TLSequencer::Initialize(const TLLocalConfig& cfg) noexcept
{
    globalBoard = new GlobalBoard<paddr_t>;

    this->config = cfg;

    std::cout << "[tl-test-passive-INFO] TLSequencer::Initialize: using seed: " << cfg.seed << std::endl;
    std::cout << "[tl-test-passive-INFO] TLSequencer::Initialize: core count: " << cfg.coreCount << std::endl;
    std::cout << "[tl-test-passive-INFO] TLSequencer::Initialize: TL-C-Agent count: " << cfg.masterCountPerCoreTLC << std::endl;
    std::cout << "[tl-test-passive-INFO] TLSequencer::Initialize: TL-UL-Agent count: " << cfg.masterCountPerCoreTLUL << std::endl;
    std::cout << "[tl-test-passive-INFO] TLSequencer::Initialize: Total agent count: " << GetAgentCount() << std::endl;

    size_t total_n_agents = GetAgentCount();

    //
    io      = new IOPort*       [total_n_agents];
    agents  = new BaseAgent*    [total_n_agents];
    fuzzers = new Fuzzer*       [total_n_agents];

    /*
    * Device ID of TileLink ports organization:
    *
    *   1. For 2-core example of 1 TL-C agent and 0 TL-UL agent:
    *       ==========================================
    *       [deviceId: 0] TL-C  Agent #0    of core #0
    *       ------------------------------------------
    *       [deviceId: 1] TL-C  Agent #0    of core #1
    *       ==========================================
    *   
    *   2. For 2-core example of 1 TL-C agent and 1 TL-UL agent:
    *       ==========================================
    *       [deviceId: 0] TL-C  Agent #0    of core #0
    *       [deviceId: 1] TL-UL Agent #0    of core #0
    *       ------------------------------------------
    *       [deviceId: 2] TL-C  Agent #0    of core #1
    *       [deviceId: 3] TL-UL Agent #0    of core #1
    *       ==========================================
    *
    *   3. For 2-core example of 1 TL-C agent and 2 TL-UL agents:
    *       ==========================================
    *       [deviceId: 0] TL-C  Agent #0    of core #0
    *       [deviceId: 1] TL-UL Agent #0    of core #0
    *       [deviceId: 2] TL-UL Agent #1    of core #0
    *       ------------------------------------------
    *       [deviceId: 3] TL-C  Agent #0    of core #1
    *       [deviceId: 4] TL-UL Agent #0    of core #1
    *       [deviceId: 5] TL-UL Agent #1    of core #1
    *       ==========================================
    *
    */

    //
    unsigned int i = 0;
    for (unsigned int j = 0; j < cfg.coreCount; j++)
    {
        for (unsigned int k = 0; k < cfg.masterCountPerCoreTLC; k++)
        {
            //
            io      [i] = new IOPort;
            agents  [i] = new CAgent(globalBoard, i, cfg.seed, &cycles);
            agents  [i]->connect(io[i]);

            //
            fuzzers [i] = new CFuzzer(static_cast<CAgent*>(agents[i]));
            fuzzers [i]->set_cycles(&cycles);

            //
            std::cout << "[tl-test-passive-INFO] TLSequencer::Initialize:" 
                <<" Instantiated TL-C Agent #" << k << " width deviceId=" << i << " for Core #" << j << "" << std::endl;

            //
            i++;
        }

        for (unsigned int k = 0; k < cfg.masterCountPerCoreTLUL; k++)
        {
            //
            io      [i] = new IOPort;
            agents  [i] = new ULAgent(globalBoard, i, cfg.seed, &cycles);
            agents  [i]->connect(io[i]);

            //
            fuzzers [i] = new ULFuzzer(static_cast<ULAgent*>(agents[i]));
            fuzzers [i]->set_cycles(&cycles);

            std::cout << "[tl-test-passive-INFO] TLSequencer::Initialize:" 
                <<" Instantiated TL-UL Agent #" << k << " width deviceId=" << i << " for Core #" << j << "" << std::endl;

            //
            i++;
        }
    }

    // IO data field pre-allocation
    for (size_t i = 0; i < total_n_agents; i++)
    {
        io[i]->a.data = make_shared_tldata<BEATSIZE>();
        io[i]->c.data = make_shared_tldata<BEATSIZE>();
        io[i]->d.data = make_shared_tldata<BEATSIZE>();
    }

    // IO field reset value
    for (size_t i = 0; i < total_n_agents; i++)
    {
        io[i]->a.ready = 0;
        io[i]->a.valid = 0;

        io[i]->b.ready = 0;
        io[i]->b.valid = 0;

        io[i]->c.ready = 0;
        io[i]->c.valid = 0;

        io[i]->d.ready = 0;
        io[i]->d.valid = 0;

        io[i]->e.ready = 0;
        io[i]->e.valid = 0;
    }

    //
    this->initialized = true;
}

void TLSequencer::Finalize() noexcept
{
    std::cout << "[TL-Test-PASSIVE] TLSequencer::TLSequencer(): finalized at cycle " << cycles << std::endl;

    for (size_t i = 0; i < GetAgentCount(); i++)
    {
        if (fuzzers[i])
        {
            delete fuzzers[i];
            fuzzers[i] = nullptr;
        }

        if (agents[i])
        {
            delete agents[i];
            agents[i] = nullptr;
        }

        if (agents[i])
        {
            delete io[i];
            io[i] = nullptr;
        }
    }

    delete globalBoard;
    globalBoard = nullptr;
}

void TLSequencer::Tick(uint64_t cycles) noexcept
{
    this->cycles = cycles;
}

void TLSequencer::Tock() noexcept
{
    if (!this->initialized)
        return;

    size_t total_n_agents = GetAgentCount();

    for (size_t i = 0; i < total_n_agents; i++)
        agents[i]->handle_channel();

    for (size_t i = 0; i < total_n_agents; i++)
        fuzzers[i]->tick();

    for (size_t i = 0; i < total_n_agents; i++)
        agents[i]->update_signal();
}

TLSequencer::IOPort* TLSequencer::IO() noexcept
{
    return *io;
}

TLSequencer::IOPort& TLSequencer::IO(int deviceId) noexcept
{
    return *(io[deviceId]);
}

void TLSequencer::FireChannelA() noexcept
{ }

void TLSequencer::FireChannelB() noexcept
{ }

void TLSequencer::FireChannelC() noexcept
{ }

void TLSequencer::FireChannelD() noexcept
{ }

void TLSequencer::FireChannelE() noexcept
{ }

void TLSequencer::UpdateChannelA() noexcept
{ }

void TLSequencer::UpdateChannelB() noexcept
{ }

void TLSequencer::UpdateChannelC() noexcept
{ }

void TLSequencer::UpdateChannelD() noexcept
{ }

void TLSequencer::UpdateChannelE() noexcept
{ }

