#pragma once

#ifndef TLC_TEST_REPORT_H
#define TLC_TEST_REPOET_H

#include "../Cover/MesPointInit.h"
#include "../Cover/Utils.h"
#include "../TLAgent/Port.h"

namespace Cover {

//---------------------------Tool----------------------------------//

static bool com_states(const int *s0, const int *s1){
    bool flag = true;
    for (int i = 0; i < N_CACHE_NUM; i++)
    {
        if(s0[i] != s1[i])
            flag = false;
    }
    return flag;
}

using namespace std;

// HLOG(P_SW_T,"chnl[%d]  op[%d]  pa[%d]  scr[%d]  core[%d]\n",it->mes[N_CH], it->mes[N_OP], it->mes[N_PA], it->mes[N_SCR], it->mes[N_CORE]);

static string ChnlToString(const int ch){
    switch(ch){
        case CHNLA: return "A";
        case CHNLB: return "B";
        case CHNLC: return "C";
        case CHNLD: return "D";
        case CHNLE: return "E";
        default: return "ilega; argc";
    }
}

using namespace tl_agent;
static string OpToString(const int chnl, const int op){
    if(chnl == CHNLA){
        switch (op){
            case PutFullData: return "PutFullData";
            case PutPartialData: return "PutPartialData";
            case Get: return "Get";
            case AcquireBlock: return "AcquireBlock";
            case AcquirePerm: return "AcquirePerm";
            default: return "ilega; argc";
        }
    }else if(chnl == CHNLB){
        return "Probe";
    }else if(chnl == CHNLC && op == ReleaseData){
        return "ReleaseData";
    }
    return "ilega; argc";
}

static string paramTostring(const int chnl, const int op, const int param){
    string s;
    if(chnl == CHNLA && (op == AcquireBlock || op == AcquirePerm)){
        switch(param){
            case NtoB: s = "NtoB"; break; 
            case NtoT: s = "NtoT"; break; 
            case BtoT: s = "BtoT"; break; 
            default: return "ilega; argc";
        }
    }else if(chnl == CHNLB){
        switch(param){
            case toN: s = "toN"; break; 
            case toB: s = "toB"; break; 
            case toT: s = "toT"; break; 
            default: return "ilega; argc";
        }
    }else if(chnl == CHNLC && op == ReleaseData){
        switch(param){
            case TtoN: s = "TtoN"; break; 
            case BtoN: s = "TtoB"; break; 
            default: return "ilega; argc";
        }
    }else{
        s = "0";
    }

    return s;
}

static string idToString(const int id){
    switch(id){
        case DCACHE_BUS_TYPE: return "D$";
        case ICACHE_BUS_TYPE: return "I$";
        case TILE_BUS_TYPE: return "L2";
        case L3_BUS_TYPE: return "L3";
        case PTW_BUS_TYPE: return "PTW";
        case DMA_BUS_TYPE: return "DMA";
        default: return "ilega; argc";
    }
}

//-------------------------Report-------------------------------//

class Report: public MesPointInit{
private:
    set<check_point> done_point;
    set<check_point> error_point;

    shared_ptr<set<check_point>> point;
    uint64_t seed;
    uint64_t cycle;
public:
    Report(uint64_t s, uint64_t c){
        point.reset(new set<check_point>);
        init("trans.txt", point);
        this->seed = s;
        this->cycle = c;
    }
    
    void record(tlMes m, cacheState bs, cacheState es){
        check_point cp;
        // mes
        cp.mes[N_CH] = m.chnl;
        cp.mes[N_OP] = m.opcode;
        cp.mes[N_PA] = m.param;
        cp.mes[N_SCR] = m.bus_type;
        cp.mes[N_CORE] = m.core_id;
        // begin states
        cp.b_states[N_CORE0_DCACHE] = bs.L1[ID_CORE0][DCACHE_BUS_TYPE];
        cp.b_states[N_CORE0_ICACHE] = bs.L1[ID_CORE0][ICACHE_BUS_TYPE];
        cp.b_states[N_CORE1_DCACHE] = bs.L1[ID_CORE1][DCACHE_BUS_TYPE];
        cp.b_states[N_CORE1_ICACHE] = bs.L1[ID_CORE1][ICACHE_BUS_TYPE];
        cp.b_states[N_CORE0_L2] = bs.L2[ID_CORE0];
        cp.b_states[N_CORE1_L2] = bs.L2[ID_CORE1];
        cp.b_states[N_L3] = bs.L3;
        // end states
        cp.e_states[N_CORE0_DCACHE] = es.L1[ID_CORE0][DCACHE_BUS_TYPE];
        cp.e_states[N_CORE0_ICACHE] = es.L1[ID_CORE0][ICACHE_BUS_TYPE];
        cp.e_states[N_CORE1_DCACHE] = es.L1[ID_CORE1][DCACHE_BUS_TYPE];
        cp.e_states[N_CORE1_ICACHE] = es.L1[ID_CORE1][ICACHE_BUS_TYPE];
        cp.e_states[N_CORE0_L2] = es.L2[ID_CORE0];
        cp.e_states[N_CORE1_L2] = es.L2[ID_CORE1];
        cp.e_states[N_L3] = es.L3;
        // addr
        cp.addr = m.address;

        std::set<check_point>::iterator it = point->find(cp);
        if(it != point->end()){
            if(com_states(it->e_states, cp.e_states)){
                done_point.insert(*it);
                point->erase(it);
            }else{
                error_point.insert(cp);

            }
        }

    }

    void print_report(){

        HLOG(P_SW_T,"\n\ndone point: [%ld]\n", done_point.size());
        for(std::set<check_point>::iterator it = done_point.begin(); it != done_point.end(); it++){
            HLOG(P_SW_T,"chnl[%s]  op[%s]  pa[%s]  scr[%s]  core[%d]\n", ChnlToString(it->mes[N_CH]).c_str(), OpToString(it->mes[N_CH], it->mes[N_OP]).c_str()
                                                                    , paramTostring(it->mes[N_CH], it->mes[N_OP], it->mes[N_PA]).c_str()
                                                                    , idToString(it->mes[N_SCR]).c_str(), it->mes[N_CORE]);
            Tool::print(it->b_states);
            Tool::print(it->e_states);
        }

        HLOG(P_SW_T,"\n\nerror point: [%ld]\n", error_point.size());
        for(std::set<check_point>::iterator it = error_point.begin(); it != error_point.end(); it++){
            HLOG(P_SW_T,"chnl[%s]  op[%s]  pa[%s]  scr[%s]  core[%d] addr[%lx]\n", ChnlToString(it->mes[N_CH]).c_str(), OpToString(it->mes[N_CH], it->mes[N_OP]).c_str()
                                                                    , paramTostring(it->mes[N_CH], it->mes[N_OP], it->mes[N_PA]).c_str()
                                                                    , idToString(it->mes[N_SCR]).c_str(), it->mes[N_CORE]
                                                                    , it->addr);
            Tool::print(it->b_states);

            std::set<check_point>::iterator correct_point = point->find(*it);
            HLOG(P_SW_T,"\nCorrect:");
            Tool::print(correct_point->e_states);
            HLOG(P_SW_T,"\nError:");
            Tool::print(it->e_states);
            // printf error point
            HLOG(P_SW_T,"%d  %d  %d  %d  %d\n", correct_point->mes[N_CH], correct_point->mes[N_OP], correct_point->mes[N_PA]
                                            , correct_point->mes[N_SCR], correct_point->mes[N_CORE]);
            for (int i = 0; i < N_CACHE_NUM; i++)
            {
                HLOG(P_SW_T,"%d  ", correct_point->b_states[i]);
            }
            HLOG(P_SW_T,"\n"); 
            for (int i = 0; i < N_CACHE_NUM; i++)
            {
                HLOG(P_SW_T,"%d  ", correct_point->e_states[i]);
            }
            HLOG(P_SW_T,"\n\n"); 
        }

        // print report
        HLOG(P_SW_T,"\n\n-----------------------TL-test report---------------------------------\n");

        // HLOG(P_SW_T,"done point:\n\n");
        // for(std::set<check_point>::iterator it = done_point.begin(); it != done_point.end(); it++){
        //     HLOG(P_SW_T,"%d  %d  %d  %d  %d\n", it->mes[N_CH], it->mes[N_OP], it->mes[N_PA]
        //                                     , it->mes[N_SCR], it->mes[N_CORE]);
        //     for (int i = 0; i < N_CACHE_NUM; i++)
        //     {
        //         HLOG(P_SW_T,"%d  ", it->b_states[i]);
        //     }
        //     HLOG(P_SW_T,"\n"); 
        //     for (int i = 0; i < N_CACHE_NUM; i++)
        //     {
        //         HLOG(P_SW_T,"%d  ", it->e_states[i]);
        //     }
        //     HLOG(P_SW_T,"\n"); 
        // }

        // HLOG(P_SW_T,"\nTo be Cover point:\n\n");
        // for(std::set<check_point>::iterator it = point->begin(); it != point->end(); it++){
        //     HLOG(P_SW_T,"%d  %d  %d  %d  %d\n", it->mes[N_CH], it->mes[N_OP], it->mes[N_PA]
        //                                     , it->mes[N_SCR], it->mes[N_CORE]);
        //     for (int i = 0; i < N_CACHE_NUM; i++)
        //     {
        //         HLOG(P_SW_T,"%d  ", it->b_states[i]);
        //     }
        //     HLOG(P_SW_T,"\n"); 
        //     for (int i = 0; i < N_CACHE_NUM; i++)
        //     {
        //         HLOG(P_SW_T,"%d  ", it->e_states[i]);
        //     }
        //     HLOG(P_SW_T,"\n"); 
        // }

        HLOG(P_SW_T,"\n\nCoverage: %f\n", (done_point.size()/(double)TOTAL_POINT));
        HLOG(P_SW_T,"Total Coverage: %f\n", ((TOTAL_POINT - point->size()) / (double)TOTAL_POINT));
        HLOG(P_SW_T,"Seed : %ld     Cycles : %ld\n\n", seed, cycle);

        // HLOG(P_SW_T,"\n\n---------------------------------------------------------------------\n\n");
        
    }

};



} // namespace Cover

#endif // TLC_TEST_REPORT_H