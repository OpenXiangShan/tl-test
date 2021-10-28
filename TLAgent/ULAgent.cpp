//
// Created by ljw on 10/21/21.
//

#include "ULAgent.h"

using namespace tl_agent;

namespace tl_agent{

    class UL_SBEntry {
    public:
        uint8_t id;
        int req_type;
        int status;
        uint64_t time_stamp;
        std::array<uint8_t, DATASIZE> data;
        UL_SBEntry(uint8_t id, int req_type, int status, uint64_t& time) {
            this->id = id;
            this->req_type = req_type;
            this->status = status;
            this->time_stamp = time;
        }
        UL_SBEntry(uint8_t id, int req_type, int status, uint64_t time, std::array<uint8_t, DATASIZE> data) {
            this->id = id;
            this->req_type = req_type;
            this->status = status;
            this->time_stamp = time;
            this->data = data;
        }
        void update_status(int status, uint64_t& time) {
            this->status = status;
            this->time_stamp = time;
        }
    };

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    class ULAgent : public BaseAgent<ReqField, RespField, EchoField, N> {
    private:
        uint64_t* cycles;
        PendingTrans<ChnA<ReqField, EchoField, N>> pendingA;
        PendingTrans<ChnD<RespField, EchoField, N>> pendingD;
        /* We only need a localBoard recording SourceID -> UL_SBEntry
         * because UL agent needn't store data.
         */
        ScoreBoard<int, UL_SBEntry> *localBoard; // SourceID -> UL_SBEntry
        void timeout_check();

    public:
        ULAgent(ScoreBoard<uint64_t, std::array<uint8_t, N>> * const gb, uint64_t* cycles);
        ~ULAgent() = default;
        Resp send_a(std::shared_ptr<ChnA<ReqField, EchoField, N>> &a);
        void handle_b();
        Resp send_c(std::shared_ptr<ChnA<ReqField, EchoField, N>> &c);
        void handle_d();
        void fire_a();
        void fire_b();
        void fire_c();
        void fire_d();
        void fire_e();
        void update();
        bool do_get(uint16_t address);
        bool do_putfulldata(uint16_t address, uint8_t data[]);
    };

    /************************** Implementation **************************/

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    Resp ULAgent<ReqField, RespField, EchoField, N>::send_a(std::shared_ptr<ChnA<ReqField, EchoField, N>> &a) {
        UL_SBEntry* entry;
        switch (*a->opcode) {
            case Get: {
                std::shared_ptr<UL_SBEntry> entry(new UL_SBEntry(*a->source, Get, S_SENDING_A, *this->cycles));
                localBoard->update(*a->source, entry);
                break;
            }
            case PutFullData: {
                std::shared_ptr<UL_SBEntry> entry(new UL_SBEntry(*a->source, PutFullData, S_SENDING_A, *this->cycles));
                localBoard->update(*a->source, entry);
                uint8_t beat_data[BEATSIZE];
                int beat_num = pendingA.nr_beat - pendingA.beat_cnt;
                for (int i = BEATSIZE * beat_num; i < BEATSIZE * (beat_num + 1); i++) {
                    this->port->a.data[i - BEATSIZE * beat_num] = a->data[i];
                }
                break;
            }
            default:
                tlc_assert(false, "Unknown opcode for channel A!");
        }
        *this->port->a.opcode = *a->opcode;
        *this->port->a.address = *a->address;
        *this->port->a.size = *a->size;
        *this->port->a.mask = *a->mask;
        *this->port->a.source = *a->source;
        *this->port->a.valid = true;
        return OK;
    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    Resp ULAgent<ReqField, RespField, EchoField, N>::send_c(std::shared_ptr<ChnA<ReqField, EchoField, N>> &c) {
        return OK;
    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    void ULAgent<ReqField, RespField, EchoField, N>::fire_a() {
        if (this->port->a.fire()) {
            *this->port->a.valid = false;
            tlc_assert(pendingA.is_pending(), "No pending A but A fired!");
            pendingA.update();
            if (!pendingA.is_pending()) { // req A finished
                localBoard->get(*pendingA.info->source)->update_status(S_WAITING_D, *cycles);
            }
        }
    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    void ULAgent<ReqField, RespField, EchoField, N>::fire_b() {

    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    void ULAgent<ReqField, RespField, EchoField, N>::fire_c() {

    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    void ULAgent<ReqField, RespField, EchoField, N>::fire_d() {
        if (this->port->d.fire()) {
            auto info = localBoard->get(*this->port->d.source);
            tlc_assert(info->status == S_WAITING_D, "Status error!");
            if (pendingD.is_pending()) { // following beats
                // TODO: wrap the following assertions into a function
                tlc_assert(*this->port->d.opcode == *pendingD.info->opcode, "Opcode mismatch among beats!");
                tlc_assert(*this->port->d.param == *pendingD.info->param, "Param mismatch among beats!");
                tlc_assert(*this->port->d.source == *pendingD.info->source, "Source mismatch among beats!");
                pendingD.update();
            } else { // new D resp
                std::shared_ptr<ChnD<RespField, EchoField, N>> resp_d(new ChnD<RespField, EchoField, N>());
                resp_d->opcode = new uint8_t(*this->port->d.opcode);
                resp_d->param = new uint8_t(*this->port->d.param);
                resp_d->source = new uint8_t(*this->port->d.source);
                resp_d->data = nullptr; // we do not care about data in PendingTrans
                int nr_beat = (*this->port->d.opcode == Grant || *this->port->d.opcode == AccessAck) ? 0 : 1; // TODO: parameterize it
                pendingD.init(resp_d, nr_beat);
            }
            if (!pendingD.is_pending()) {
                // ULAgent needn't care about endurance
                localBoard->erase(*this->port->d.source);
                this->idpool.freeid(*this->port->d.source);
            }
        }
    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    void ULAgent<ReqField, RespField, EchoField, N>::fire_e() {

    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    void ULAgent<ReqField, RespField, EchoField, N>::handle_b() {

    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    void ULAgent<ReqField, RespField, EchoField, N>::handle_d() {

    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    ULAgent<ReqField, RespField, EchoField, N>::ULAgent(ScoreBoard<uint64_t, std::array<uint8_t, N>> *gb, uint64_t* cycles):
        BaseAgent<ReqField, RespField, EchoField, N>(), pendingA(), pendingD()
    {
        this->globalBoard = gb;
        this->cycles = cycles;
        localBoard = new ScoreBoard<int, UL_SBEntry>();
    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    void ULAgent<ReqField, RespField, EchoField, N>::update() {
        fire_a();
        fire_d();

        *this->port->d.ready = true; // TODO: do random here

        if (pendingA.is_pending()) {
            // TODO: do delay here
            send_a(pendingA.info);
        }
        // do timeout check lazily
        if (*this->cycles % TIMEOUT_INTERVAL == 0) {
            this->timeout_check();
        }
    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    bool ULAgent<ReqField, RespField, EchoField, N>::do_get(uint16_t address) {
        if (pendingA.is_pending())
            return false;
        std::shared_ptr<ChnA<ReqField, EchoField, N>> req_a(new ChnA<ReqField, EchoField, N>());
        req_a->opcode = new uint8_t(Get);
        req_a->address = new uint16_t(address);
        req_a->size = new uint8_t(ceil(log2((double)DATASIZE)));
        req_a->mask = new uint32_t(0xffffffffUL);
        req_a->source = new uint8_t(this->idpool.getid());
        pendingA.init(req_a, 1);
        return true;
    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    bool ULAgent<ReqField, RespField, EchoField, N>::do_putfulldata(uint16_t address, uint8_t data[]) {
        if (pendingA.is_pending())
            return false;
        std::shared_ptr<ChnA<ReqField, EchoField, N>> req_a(new ChnA<ReqField, EchoField, N>());
        req_a->opcode = new uint8_t(PutFullData);
        req_a->address = new uint16_t(address);
        req_a->size = new uint8_t(ceil(log2((double)DATASIZE)));
        req_a->mask = new uint32_t(0xffffffffUL);
        req_a->source = new uint8_t(this->idpool.getid());
        req_a->data = data;
        pendingA.init(req_a, DATASIZE / BEATSIZE);
        return true;
    }

    template<class ReqField, class RespField, class EchoField, std::size_t N>
    void ULAgent<ReqField, RespField, EchoField, N>::timeout_check() {
        if (localBoard->mapping.empty()) {
            return;
        }
        for (auto it = this->localBoard->mapping.begin(); it != this->localBoard->mapping.end(); it++) {
            auto value = it->second;
            if (value->status != S_INVALID && value->status != S_VALID) {
                if (*this->cycles - value->time_stamp > TIMEOUT_INTERVAL) {
                    printf("Now time:   %llu\n", *this->cycles);
                    printf("Last stamp: %llu\n", value->time_stamp);
                    printf("Status:     %d\n", value->status);
                    tlc_assert(false,  "Transaction time out");
                }
            }
        }
    }
}