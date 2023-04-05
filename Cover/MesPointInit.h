#ifndef TLC_TEST_COVERPOINTINIT_H
#define TLC_TEST_COVERPOINTINIT_H

#include "../Monitor/Tool.h"
#include <set>
#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<sstream>

namespace Cover{

enum{
    N_MES = 5,
    N_CACHE_NUM = 7,

    N_CH = 0,// channel
    N_OP,// opcode
    N_PA,// param
    N_SCR,
    N_CORE,
    N_BS,// begin states
    N_ES = N_BS + N_CACHE_NUM,// end states
};
enum{
    // Cache
    N_NONE = -1,
    N_CORE0_DCACHE = 0,
    N_CORE0_ICACHE = 1,
    N_CORE1_DCACHE = 2,
    N_CORE1_ICACHE = 3,
    N_CORE0_L2 = 4,
    N_CORE1_L2 = 5,
    N_L3 = 6,
};
enum{
    TOTAL_POINT = 1776,
};

class check_point{
public:
    int mes[N_MES];
    int b_states[N_CACHE_NUM];
    int e_states[N_CACHE_NUM];  
    paddr_t addr;

    bool operator<(const check_point& cp) const{
        // mes
        for (int i = 0; i < N_MES; i++)
        {
            if(mes[i] != cp.mes[i]){
                return mes[i] < cp.mes[i];
            }
        }
        // b_states
        for (int i = 0; i < N_CACHE_NUM; i++)
        {
            if(b_states[i] != cp.b_states[i]){
                return b_states[i] < cp.b_states[i];
            }
        }
        // e_states
        // for (int i = 0; i < N_CACHE_NUM; i++)
        // {
        //     if(e_states[i] != cp.e_states[i]){
        //         return e_states[i] < cp.e_states[i];
        //     }
        // }
        // if all equal
        return e_states[N_CACHE_NUM-1] < cp.e_states[N_CACHE_NUM-1];
    }

};

using namespace std;

class MesPointInit
{
private:
    const int base_long = N_MES + N_CACHE_NUM*2;
    const long arr_size = (TOTAL_POINT+1) * (N_MES + N_CACHE_NUM*2);
public:
    void init(string infile, shared_ptr<set<check_point>> point){
        string arr[arr_size];
        //read txt
        vector<string> vec;
        ifstream input(infile);
        if (input)
        {
            string word;
            while (input >> word) {
            vec.push_back(word); // Insert the element at the end of the vector
            }
        }
        else {
            cout << "Could not open the file " << endl;
            return;
        }//Output Could not open file if not inserted successfully
        input.close();

        //save data
        int n = vec.size(); 
        cout << "data size:" << n << endl;
        for (int i = 0; i < n; i++)
        {
            arr[i] = vec[i];
            // cout << vec[i] << endl;
        }

        // sort
        long int num = 0;
        for (long i = 0; i < n; i+=base_long)
        {
            check_point cp;
            for (int i0 = 0; i0 < N_MES; i0++)
            {
                cp.mes[i0] = stoi(arr[i+i0]);
            }
            for (int i1 = N_MES; i1 < N_MES+N_CACHE_NUM; i1++)
            {
                cp.b_states[i1-N_MES] = stoi(arr[i+i1]);
            }
            for (int i2 = N_MES+N_CACHE_NUM; i2 < base_long; i2++)
            {
                cp.e_states[i2-(N_MES+N_CACHE_NUM)] = stoi(arr[i+i2]);
            }  
            point->insert(cp);
            num++;
        }
        
        // for(std::set<check_point>::iterator it = point.begin(); it != point.end(); it++){
        //     printf("%d  %d  %d  %d  %d\n",it->mes[N_CH], it->mes[N_OP], it->mes[N_PA], it->mes[N_SCR], it->mes[N_CORE]);
        //     Tool::print(it->b_states);
        //     Tool::print(&it->e_states[0]);
        // }
        
        printf("Input Trans num = %ld\n", num);
        printf("%s Input Trans Success!\n", infile.c_str());
    }
};




};
 // namespace Cover

#endif // TLC_TEST_COVERPOINTINIT_H