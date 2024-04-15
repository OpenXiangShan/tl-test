//
//
//

#include "TLSequencer.hpp"


TLSequencer::TLSequencer() noexcept
    : globalBoard   (nullptr)
    , agents        (new BaseAgent*[NR_AGENTS])
    , fuzzers       (new Fuzzer*[NR_AGENTS])
    , io            (new IOPort*[NR_AGENTS])
    , cycles        (0)
    , seed          (0)
{ }

TLSequencer::~TLSequencer() noexcept
{
    delete[] fuzzers;
    delete[] agents;
    delete[] io;
}

void TLSequencer::Initialize() noexcept
{
    globalBoard = new GlobalBoard<paddr_t>;

    std::cout << "[TL-Test-PASSIVE] TLSequencer::TLSequencer(): using seed: " << seed << std::endl;
    std::cout << "[TL-Test-PASSIVE] TLSequencer::TLSequencer(): NR_ULAGENTS = " << NR_ULAGENTS << std::endl;
    std::cout << "[TL-Test-PASSIVE] TLSequencer::TLSequencer(): NR_CAGENTS = " << NR_CAGENTS << std::endl;

    for (int i = 0; i < NR_ULAGENTS; i++)
    {
        //
        io      [i] = new IOPort;
        agents  [i] = new ULAgent(globalBoard, i, seed, &cycles);
        agents  [i]->connect(io[i]);

        //
        fuzzers [i] = new ULFuzzer(static_cast<ULAgent*>(agents[i]));
        fuzzers [i]->set_cycles(&cycles);
    }

    for (int i = NR_ULAGENTS; i < NR_AGENTS; i++)
    {
        //
        io      [i] = new IOPort;
        agents  [i] = new CAgent(globalBoard, i, seed, &cycles);
        agents  [i]->connect(io[i]);

        //
        fuzzers [i] = new CFuzzer(static_cast<CAgent*>(agents[i]));
        fuzzers [i]->set_cycles(&cycles);
    }

    // IO data field pre-allocation
    for (int i = 0; i < NR_AGENTS; i++)
    {
        io[i]->a.data = make_shared_tldata<BEATSIZE>();
        io[i]->c.data = make_shared_tldata<BEATSIZE>();
        io[i]->d.data = make_shared_tldata<BEATSIZE>();
    }

    // IO field reset value
    for (int i = 0; i < NR_AGENTS; i++)
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
}

void TLSequencer::Finalize() noexcept
{
    std::cout << "[TL-Test-PASSIVE] TLSequencer::TLSequencer(): finalized at cycle " << cycles << std::endl;

    for (int i = 0; i < NR_AGENTS; i++)
    {
        delete fuzzers[i];
        fuzzers[i] = nullptr;

        delete agents[i];
        agents[i] = nullptr;

        delete io[i];
        io[i] = nullptr;
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
    for (int i = 0; i < NR_AGENTS; i++)
        agents[i]->handle_channel();

    for (int i = 0; i < NR_AGENTS; i++)
        fuzzers[i]->tick();

    for (int i = 0; i < NR_AGENTS; i++)
        agents[i]->update_signal();
}

TLSequencer::IOPort* TLSequencer::IO() noexcept
{
    return *io;
}

TLSequencer::IOPort& TLSequencer::IO(int deviceId) noexcept
{
    return (*io)[deviceId];
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

