//
// Created by dxy on 2023/4.
//

#ifndef TLC_TEST_DIR_WRITE_H
#define TLC_TEST_DIR_WRITE_H

#include "../Utils/Common.h"
#include "../Interface/Interface.h"
#include "../TLAgent/Port.h"


namespace dir_write{

    class DIR_Write
    {
    private:
        uint64_t id;
        uint8_t bus_type;//7
        std::shared_ptr<tl_interface::DIRInfo> info;
    public:
        DIR_Write(uint64_t iid, uint8_t bt);
        ~DIR_Write() {};

        void test();
        void close_test(){this->info->arbiter.reset(new uint8_t(DIR_NOT_WRITE));}
    };
    

}


#endif // TLC_TEST_DIR_WRITE_H