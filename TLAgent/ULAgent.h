//
// Created by ljw on 10/21/21.
//

#ifndef TLC_TEST_ULAGENT_H
#define TLC_TEST_ULAGENT_H

#include "BaseAgent.h"
#include "../Utils/Common.h"
#include "../Utils/ScoreBoard.h"

namespace tl_agent {

    class UL_SBEntry {
    public:
        paddr_t address;
        int req_type;
        int status;
        uint64_t time_stamp;
        std::array<uint8_t, DATASIZE> data;
        UL_SBEntry(int req_type, int status, paddr_t address, uint64_t& time) {
            this->req_type = req_type;
            this->status = status;
            this->address = address;
            this->time_stamp = time;
        }
        UL_SBEntry(int req_type, int status, paddr_t address, uint64_t time, std::array<uint8_t, DATASIZE> data) {
            this->req_type = req_type;
            this->status = status;
            this->address = address;
            this->time_stamp = time;
            this->data = data;
        }
        void update_status(int status, uint64_t& time) {
            this->status = status;
            this->time_stamp = time;
        }
    };

    class ULAgent : public BaseAgent {
    private:
        uint64_t* cycles;
        PendingTrans<ChnA<ReqField, EchoField, DATASIZE>> pendingA;
        PendingTrans<ChnD<RespField, EchoField, DATASIZE>> pendingD;
        /* We only need a localBoard recording SourceID -> UL_SBEntry
         * because UL agent needn't store data.
         */
        ScoreBoard<int, UL_SBEntry> *localBoard; // SourceID -> UL_SBEntry
        void timeout_check();

    public:
        ULAgent(GlobalBoard<paddr_t> * const gb, int id, uint64_t* cycles);
        ~ULAgent() = default;
        Resp send_a(std::shared_ptr<ChnA<ReqField, EchoField, DATASIZE>> &a);
        void handle_b(std::shared_ptr<ChnB> &b);
        Resp send_c(std::shared_ptr<ChnC<ReqField, EchoField, DATASIZE>> &c);
        void fire_a();
        void fire_b();
        void fire_c();
        void fire_d();
        void fire_e();
        void handle_channel();
        void update_signal();
        bool do_getAuto(paddr_t address);
        bool do_get(paddr_t address, uint8_t size, uint32_t mask);
        bool do_putfulldata(paddr_t address, uint8_t data[]);
        bool do_putpartialdata(uint16_t address, uint8_t size, uint32_t mask, uint8_t data[]);
    };

}

#endif //TLC_TEST_ULAGENT_H
