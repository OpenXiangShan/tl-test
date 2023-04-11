//
// Created by dxy on 2023/2/13.
//

#ifndef TLC_TEST_CASE_WITH_STATES_H
#define TLC_TEST_CASE_WITH_STATES_H

#include "../Utils/Common.h"
#include "../TLAgent/Port.h"
#include <map>
#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<sstream>
#include "../Cover/Utils.h"
#include "../Monitor/Tool.h"

namespace testcase_with_states {

    enum{
        n_agent_id= 0,
        n_core_id,
        n_chnl,
        n_opcode,
        n_param,
        n_max,

        n_self = 0,
        n_self_client0,
        n_self_client1,
        n_client0,
        n_client1,
        n_states,

        n_cache_states = 7,
    };

    enum{
        reset_opcode = 16
    };

    enum{
        TOTAL_CASE = 1576,
    };

    enum{
        base_cycle = 9000,
        cycle_step = 9000,
        cycle_done = 500,
    };

    class states{
    public:
        uint8_t self[3];// CORE0 L2 CORE1 L2 L3
        uint8_t self_client[3][2];// CORE0 L2 CORE1 L2 L3 -> client0 client1
        uint8_t client[3][2];// CORE0 L2 CORE1 L2 L3 -> client0 client1
    };

    //cycle agent_id opcode param DIR_states
    class Mes_Entry{
    public:
        uint8_t agentid;
        uint8_t core_id;
        uint8_t chnl;
        uint8_t opcode;
        uint8_t param;
        states state;
        uint8_t begin_states[n_cache_states];
        uint8_t end_states[n_cache_states];
    };

    //cycle,Entry
    class Case_with_satets {
    protected:
        const long arr_size = TOTAL_CASE*(n_max + n_states*3 + n_cache_states*2) + 1;
        const long base_long = n_max + n_states*3 + n_cache_states*2;
        
    public:
        std::map<uint64_t, Mes_Entry> tc;//test case
        std::map<uint64_t, Mes_Entry> tc_error;//test case
        uint64_t num = 0;
        uint64_t done = 0;
        uint64_t error = 0;

        Case_with_satets(){};
        ~Case_with_satets(){};

        void read_test(std::string infile, bool re);

        void check(uint64_t cycle, uint8_t *cache_state){
            bool flag;
            for (uint8_t i = 0; i < n_cache_states; i++)
            {
                if(tc[cycle].end_states[i] != cache_state[i])
                    flag = false;
            }

            uint8_t s[n_cache_states];
            using namespace Tool;
            // Begin
            for (uint8_t i = 0; i < n_cache_states; i++)
            {
                s[i] = tc[cycle].begin_states[i];
            }
            HLOG(P_SW,"\n[%s]-[%s]  [%s]-[%s]\n" , stateTostring(s[0]).c_str() , stateTostring(s[1]).c_str() 
                                            , stateTostring(s[2]).c_str() , stateTostring(s[3]).c_str() );
            HLOG(P_SW,"  [%s]      [%s]\n"    , stateTostring(s[4]).c_str(), stateTostring(s[5]).c_str() );
            HLOG(P_SW,"       [%s]\n\n"         , stateTostring(s[6]).c_str() );
            // End
            for (uint8_t i = 0; i < n_cache_states; i++)
            {
                s[i] = tc[cycle].end_states[i];
            }
            HLOG(P_SW,"\n[%s]-[%s]  [%s]-[%s]\n" , stateTostring(s[0]).c_str() , stateTostring(s[1]).c_str() 
                                            , stateTostring(s[2]).c_str() , stateTostring(s[3]).c_str() );
            HLOG(P_SW,"  [%s]      [%s]\n"    , stateTostring(s[4]).c_str(), stateTostring(s[5]).c_str() );
            HLOG(P_SW,"       [%s]\n\n"         , stateTostring(s[6]).c_str() );


            if(flag){
                HLOG(P_SW,"CHECK TRUE!\n\n");
                // erase
                if(tc.count(cycle) > 0 && tc.count(cycle+cycle_done) > 0){
                    tc.erase(cycle);
                    tc.erase(cycle+cycle_done);
                }
                else{
                    tlc_assert(false,"no cycle\n");
                }
                done++;
            }else{
                HLOG(P_SW,"CHECK FALSE!\n\n");
                // record error
                if(tc.count(cycle) > 0 && tc.count(cycle+cycle_done) > 0){
                    tc_error.insert(std::make_pair(error++,tc[cycle]));
                    tc.erase(cycle);
                    tc.erase(cycle+cycle_done);
                }

                for (uint8_t i = 0; i < n_cache_states; i++)
                {
                    s[i] = cache_state[i];
                }
                HLOG(P_SW,"\n[%s]-[%s]  [%s]-[%s]\n" , stateTostring(s[0]).c_str() , stateTostring(s[1]).c_str() 
                                                , stateTostring(s[2]).c_str() , stateTostring(s[3]).c_str() );
                HLOG(P_SW,"  [%s]      [%s]\n"    , stateTostring(s[4]).c_str(), stateTostring(s[5]).c_str() );
                HLOG(P_SW,"       [%s]\n\n"         , stateTostring(s[6]).c_str() );
            }
        }

        void print_report(){

            uint64_t n = 0;
            for(auto [key, val] : tc){
                if(val.opcode != reset_opcode){
                    HLOG(P_SW,"%d %d %d %d %d\n", val.agentid, val.core_id, val.chnl, val.opcode, val.param);
                    for (uint8_t i = 0; i <3; i++)
                    {
                        HLOG(P_SW,"%d %d %d %d %d\n", val.state.self[3], val.state.self_client[i][0], val.state.self_client[i][1]
                                                                                , val.state.client[i][0], val.state.client[i][1]);
                    }
                    for (uint8_t id = 0; id < n_cache_states; id++)
                    {
                        HLOG(P_SW,"%d ", val.begin_states[id]);
                    }
                    HLOG(P_SW,"\n");
                    for (uint8_t id = 0; id < n_cache_states; id++)
                    {
                        HLOG(P_SW,"%d ", val.end_states[id]);
                    }
                    HLOG(P_SW,"\n");
                    n++;
                }
            }

            HLOG(P_SW_T,"\nnum[%ld] n[%ld] done[%ld]\n", num, n, done);
            HLOG(P_SW_T,"\n\nCoverage: %f\n", ((num-tc.size()/2)/(double)TOTAL_CASE));
            HLOG(P_SW_T,"Total Coverage: %f\n", ((TOTAL_CASE-tc.size()/2)/(double)TOTAL_CASE));

            HLOG(P_SW_T,"\n\n---------------------------------------------------------------------\n\n");

            HLOG(P_SW_T,"\nerror_num[%ld]\n", error);

            for(auto [key, val] : tc_error){
                if(val.opcode != reset_opcode){
                    HLOG(P_SW,"%d %d %d %d %d\n", val.agentid, val.core_id, val.chnl, val.opcode, val.param);
                    for (uint8_t i = 0; i <3; i++)
                    {
                        HLOG(P_SW,"%d %d %d %d %d\n", val.state.self[3], val.state.self_client[i][0], val.state.self_client[i][1]
                                                                                , val.state.client[i][0], val.state.client[i][1]);
                    }
                    for (uint8_t id = 0; id < n_cache_states; id++)
                    {
                        HLOG(P_SW,"%d ", val.begin_states[id]);
                    }
                    HLOG(P_SW,"\n");
                    for (uint8_t id = 0; id < n_cache_states; id++)
                    {
                        HLOG(P_SW,"%d ", val.end_states[id]);
                    }
                    HLOG(P_SW,"\n");
                    n++;
                }
            }
        
        }
    };

}

#endif //TLC_TEST_CASE_WITH_STATES_H
