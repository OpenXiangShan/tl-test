//
// Created by dxy on 2023/3.
//

#ifndef TLC_TEST_MESCOLLECT_H
#define TLC_TEST_MESCOLLECT_H


#include "../Utils/Common.h"
#include "../TLAgent/Port.h"
#include "../Monitor/DIR_Monitor.h"
#include <map>
#include "../Cover/MesCom.h"

namespace Cover {

class storage{
public:
    tlMes mes;
    uint64_t n;
    bool dir;// dir be write
    bool tag;// tag be write
};

// waiting state to be writed in L2 & L3
// The time to write to directory is delayed from the time of ACK is sent
// build a pool to storage the message which is wating to be writed in
class waiting_pool{
private:
    // TODO:
    // This is a risky operation because 
    // Other operations within max_cycle 
    // may also rewrite the DIR
    const uint64_t max_cycle = 15;
    std::map<paddr_t,storage> self_pool[ID_MAX-1];//[id]id:0->core0 L2; 1->core1 L2; 2->L3
    std::map<paddr_t,storage> client_pool[ID_MAX-1];
public:

    void add_wating(paddr_t key, uint8_t id, tlMes mes){
      storage st;
      st.mes = mes;
      st.n = max_cycle;
      st.dir = false;
      st.tag = false;
      self_pool[id].insert({key,st});
      client_pool[id].insert({key,st});
      printf("self waiting cycle:[%lx] [%ld]\n", key, self_pool[id][key].n);
      printf("client waiting cycle:[%lx] [%ld]\n", key, client_pool[id][key].n);
    }

    bool self_haskey(paddr_t key, uint8_t id){
      if(self_pool[id].count(key) > 0)
        return true;
      else
        return false;
    }

    bool client_haskey(paddr_t key, uint8_t id){
      if(client_pool[id].count(key) > 0)
        return true;
      else
        return false;
    }

    void self_earse_DirOrTag(uint8_t DirOrTag, paddr_t key, uint8_t id){
      if(self_pool[id].count(key) > 0){
        if(DirOrTag == DIR_monitor::DIR)
          self_pool[id][key].dir = true;
        else if(DirOrTag == DIR_monitor::TAG)
          self_pool[id][key].tag = true;
        else
          tlc_assert(false,"client_earse_DirOrTag error!\n");
      }else{
        tlc_assert(false,"client_earse_DirOrTag error!\n");tlc_assert(false,"client_earse_DirOrTag error!\n");
      }
    }

    bool self_be_write_finish(paddr_t key, uint8_t id){
      if(self_pool[id].count(key) > 0){
        if(self_pool[id][key].dir && self_pool[id][key].tag)
          return true;
        else
          return false;
      }else{
        tlc_assert(false,"self_be_write_finish error!\n");
      }
    }

    void client_earse_DirOrTag(uint8_t DirOrTag, paddr_t key, uint8_t id){
      if(client_pool[id].count(key) > 0){
        if(DirOrTag == DIR_monitor::DIR)
          client_pool[id][key].dir = true;
        else if(DirOrTag == DIR_monitor::TAG)
          self_pool[id][key].tag = true;
        else
          tlc_assert(false,"client_earse_DirOrTag error!\n");
      }else{
        tlc_assert(false,"client_earse_DirOrTag error!\n");tlc_assert(false,"client_earse_DirOrTag error!\n");
      }
    }

    bool client_be_write_finish(paddr_t key, uint8_t id){
      if(client_pool[id].count(key) > 0){
        if(client_pool[id][key].dir && client_pool[id][key].tag)
          return true;
        else
          return false;
      }else{
        tlc_assert(false,"self_be_write_finish error!\n");
      }
    }

    void erase_self_wating(paddr_t key, uint8_t id){
      if(self_pool[id].count(key) > 0)
        self_pool[id].erase(key);
    }

    tlMes get_self(paddr_t key, uint8_t id){
      if(self_pool[id].count(key) > 0)
        return self_pool[id][key].mes;
      else
        tlc_assert(false,"Self DIR POOL no key!\n");
    }

    tlMes get_client(paddr_t key, uint8_t id){
      if(client_pool[id].count(key) > 0)
        return client_pool[id][key].mes;
      else
        tlc_assert(false,"CLIENT DIR POOL no key!\n");
    }

    void erase_client_wating(paddr_t key, uint8_t id){
      if(client_pool[id].count(key) > 0)
        client_pool[id].erase(key);
    }

    tlMes check_time(void){
      //INIT
      tlMes mes;
      mes.address = 0x0;
    
      for (size_t id = 0; id < ID_MAX-1; id++)
      {
        if(self_pool[id].size() > 0){
            for (auto it = self_pool[id].begin(); it != self_pool[id].end(); it++) {
              paddr_t key = it->first;
              storage val = it->second;
                if(val.n <= 1){
                    mes = self_pool[id][key].mes;
                    erase_self_wating(key, id);
                    erase_client_wating(key, id);
                    printf("erase SELF wating: [%ld] [%lx]\n",id ,mes.address);
                    return mes;
                }
            }
        } 
        if(client_pool[id].size() > 0){
            for (auto it = client_pool[id].begin(); it != client_pool[id].end(); it++) {
              paddr_t key = it->first;
              storage val = it->second;
                if(val.n <= 1){
                    mes = self_pool[id][key].mes;
                    erase_self_wating(key, id);
                    erase_client_wating(key, id);
                    printf("erase CLIENT wating: [%ld] [%lx]\n",id ,mes.address);
                    return mes;
                }
            }
        }
      }
      return mes;
    }    

    void update_time(void){
      //INIT
      storage st;
    
      for (size_t id = 0; id < ID_MAX-1; id++)
      {
        if(self_pool[id].size() > 0){
            for (auto it = self_pool[id].begin(); it != self_pool[id].end(); it++) {
              paddr_t key = it->first;
              storage val = it->second;
                if(val.n == 0){
                    printf("ADDR:%lx ",key);
                    tlc_assert(false,"Self DIR POOL write time out!\n");
                }else{
                    st.mes = val.mes;
                    st.n = val.n-1;
                    self_pool[id][key] = st;
                    // printf("SELF: id:%ld key:%lx val:%ld\n",id, key, st.n);
                }
            }
        } 
        if(client_pool[id].size() > 0){
            for (auto it = client_pool[id].begin(); it != client_pool[id].end(); it++) {
              paddr_t key = it->first;
              storage val = it->second;
                if(val.n == 0){
                    printf("ADDR:%lx ",key);
                    tlc_assert(false,"Self DIR POOL write time out!\n");
                }else{
                    st.mes = val.mes;
                    st.n = val.n-1;
                    self_pool[id][key] = st;
                    // printf("SELF: id:%ld key:%lx val:%ld\n",id, key, st.n);
                }
            }
        }
      }
    }
};


class Mes_Collect{
private:

    Mes_Com *mes_com;

    std::shared_ptr<tl_interface::TLInfo> tl_info;

    // Requests -> read DIR immediately
    // Grant -> no need to read DIR
    // ACK in addition to Grant -> Put in Pool waiting state to be written 
    waiting_pool pool;
    
    // message send to Compare
    tlMes Mes;
    cacheState State;

    //self DIR
    ScoreBoard<DIR_monitor::Dir_key,DIR_monitor::Dir_Mes> *Self_Dir_Storage;//2*L2 + L3;
    ScoreBoard<DIR_monitor::Dir_key,paddr_t> *Self_Dir_Tag_Storage;//2*L2 + L3;
    //client DIR
    ScoreBoard<DIR_monitor::Dir_key,DIR_monitor::Dir_Mes> *Client_Dir_Storage;//2*L2 + L3;
    ScoreBoard<DIR_monitor::Dir_key,paddr_t> *Client_Dir_Tag_Storage;//2*L2 + L3;

    // mapping for source&sink to address
    // core0  core1 
    //  L1   L1
    //   |   |              ->3*2=6     source[bus_type][core_id][channel]: source->address      Channel: A B C
    //  L2   L2  DMA                    sink[bus_type][core_id]:            sink->address
    //   \   |  /           ->3
    //       L3
    //       |              ->1
    //      MEM
    //
    // 4 -> L3slice 0 1 2 3 (core0 core1)
    // 3 -> CHNLA B C
    //
    std::map<uint8_t,paddr_t> source[AGENT_BUS_TYPE_MAX][4][3];
    std::map<uint8_t,paddr_t> sink[AGENT_BUS_TYPE_MAX][4];

    // Processing and packaging for TL-Link information
    void handle_ChnlA_info(uint64_t core_id, uint8_t bus_type);
    void handle_ChnlB_info(uint64_t core_id, uint8_t bus_type);
    void handle_ChnlC_info(uint64_t core_id, uint8_t bus_type);
    void handle_ChnlD_info(uint64_t core_id, uint8_t bus_type);
    void handle_ChnlE_info(uint64_t core_id, uint8_t bus_type);
    // Processing and packaging for Cache State
    cacheState get_state_info(paddr_t addr);
public:   
    Mes_Collect(ScoreBoard<DIR_monitor::Dir_key,DIR_monitor::Dir_Mes> *const selfDir, ScoreBoard<DIR_monitor::Dir_key,paddr_t> *const selfTag
              ,ScoreBoard<DIR_monitor::Dir_key,DIR_monitor::Dir_Mes> *const clientDir, ScoreBoard<DIR_monitor::Dir_key,paddr_t> *const clientTag
              ,Mes_Com *const mc);

    void fire_Mes_Collect(const std::shared_ptr<tl_interface::TLInfo> info,uint64_t core_id, uint8_t bus_type);

    // pool
    void check_time_out(void);
    void update_pool(paddr_t addr, uint64_t dir_id, bool DIR, uint8_t DirOrTag);

    // send package to Mes_com
    void send(bool state_valid);
};


} // namespace Cover

#endif // TLC_TEST_MESCOLLECT_H