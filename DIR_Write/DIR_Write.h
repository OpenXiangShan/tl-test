//
// Created by dxy on 2023/4.
//

#ifndef TLC_TEST_DIR_WRITE_H
#define TLC_TEST_DIR_WRITE_H

#include "../Utils/Common.h"
#include "../Interface/Interface.h"
#include "../TLAgent/Port.h"
#include "../Sequencer/Case_with_states.h"
#include "../Monitor/Tool.h"
#include "../Monitor/DIR_Monitor.h"


namespace dir_write{

    class DIR_Write
    {
    private:
        enum{
            L2_0 = 0,
            L2_1,
            L3,
            MAX,
        };
        uint64_t id;
        uint8_t bus_type;//7
        std::shared_ptr<tl_interface::DIRInfo> info[MAX];// L2*2 + L3
    public:
        DIR_Write(uint8_t bt);
        ~DIR_Write() {};

        void output(uint64_t id, paddr_t self_tag, paddr_t client_tag, paddr_t set, uint8_t way, uint8_t self, uint8_t client0, uint8_t client1);
        void close(){
                for (uint64_t i = 0; i < MAX; i++)
                {
                    this->info[i]->arbiter.reset(new uint8_t(DIR_NOT_WRITE));
                }
            }
        void write(testcase_with_states::states state, paddr_t addr);
    };
    

}


#endif // TLC_TEST_DIR_WRITE_H