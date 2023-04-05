#ifndef TLC_TEST_MESCOM_H
#define TLC_TEST_MESCOM_H

#include "../Monitor/Tool.h"
#include "../Utils/Common.h"
#include "../TLAgent/Port.h"
#include "../Cover/Report.h"
#include <set>

namespace Cover {

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

    bool operator!=(const link_index& key)const {
        if(this->bus_type == key.bus_type){
            return this->core_id != key.core_id;
        }else{
            return this->bus_type != key.bus_type;
        }
    }

    bool operator==(const link_index& key)const {
        if(this->bus_type == key.bus_type){
            return this->core_id == key.core_id;
        }else{
            return false;
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
    std::map<link_index,link_col> link;
    link_index first_col_index;
    bool first_in_link = true;
public:
    void reset(){
        link.clear(); 
        first_col_index.bus_type = AGENT_BUS_TYPE_MAX; 
        first_col_index.core_id = ID_MAX; 
        first_in_link = true;
    }

    // TODO: collet all message 
    void update_all(package pk);

    // only collect the first message link
    bool updata_first(package pk);

    bool check_finish();
    void print();
    link_col get_first_col(){
        return link[first_col_index];
    }
};


class Mes_Com{
private:
    Report *report;
    std::map<paddr_t, link> queue;
public:
    Mes_Com(Report *const rp){
        this->report = rp;
    };
    void arbiter(package pk);
    
};




} // namespace Cover

#endif // TLC_TEST_MESCOM_H