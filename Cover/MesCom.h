#ifndef TLC_TEST_MESCOM_H
#define TLC_TEST_MESCOM_H


#include "../Utils/Common.h"
#include "../TLAgent/Port.h"
#include "../Cover/Utils.h"

namespace Cover {

// TODO: now only support request with different addresses recording
// class com_key{
// public:
    //paddr_t addr;
    // uint8_t n;

    // bool operator<(const com_key& key)const {
    //     if(this->addr == key.addr){
    //         return this->n < key.n;
    //     }else{
    //         return this->addr < key.addr;
    //     }
    // }
// };

class link_index{
public:
    uint64_t core_id;//only work for L1 L2
    uint8_t bus_type;

    bool operator<(const link_index& key)const {
        if(this->bus_type == key.bus_type){
            return this->core_id < key.core_id;
        }else{
            return this->bus_type < key.bus_type;
        }
    }
};

class link_col{
public:
    tlMes Mes_Req;
    cacheState State_b;//begin
    tlMes Mes_Ack;
    cacheState State_e;//end
    tlMes Mes_Ack1;//For GrantAck
};


// TODO: now only support one request transfer recording between two modules
class link{
private:
    //mes type
    enum{
        REQ = 0,
        ACK = 1,
        ACK1 = 2,
    };
    std::map<link_index,link_col> link;
    link_index first_col_index;
public:
    void reset(){link.clear();}
    void update(package pk);
    bool check_finish();
    void print();
};


class Mes_Com{
private:
    std::map<paddr_t, link> queue;
public:
    void arbiter(package pk);
};




} // namespace Cover

#endif // TLC_TEST_MESCOM_H