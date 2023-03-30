#ifndef TLC_TEST_TOOL_H
#define TLC_TEST_TOOL_H

#include "../TLAgent/Port.h"

#define GetBit(v, n) (((v) & ((uint64_t)1 << (n)))>0) //Get the n bit of v
#define SetBit(v, n) ((v) |= ((uint64_t)1 << (n)))//Set the n bit of v to 1
#define ClearBit(v, n) ((v) &= (~(uint64_t)1 << (n)))//Set the n bit of v to 0

namespace Tool{

//------------------------------------Tool---------------------------------------------//    
    
    //get bit from begin to end, the first bit_index is 0
    static paddr_t Get_n_bit(paddr_t addr, uint8_t begin, uint8_t end){
        paddr_t key = 0;
        for (int i = end; i >= begin; i--)
        {
            key = (key<<1) + GetBit(addr,i);
            //printf("bit: %d \n", GetBit(addr,i));
        }
        //printf("value: %lx \n", key);
        return key;
    }

    using namespace std;

    static string hex_to_str(uint64_t mask,int len,bool x){
        string hexes[16] = {"0","1","2","3","4","5","6","7","8","9","a","b","c","d","e","f"};
        string hexstring = "";
        for (int i=0; i<len; i++){
        int j = len-i-1;
        int number = (mask >> 4*j) & 0xf;
        hexstring += hexes[number];
        }
        if(x == true)
        return "0x" + hexstring;
        else 
        return hexstring;
    }

    using namespace tl_agent;

    static string stateTostring(uint8_t state){
        string s;
        switch (state)
        {
        case TIP:
            s = "TT";
            break;
        case TRUNK:
            s = "T";
            break;
        case BRANCH:
            s = "B";
            break;
        case INVALID:
            s = "N";
            break;
        default: s = "error" + to_string(state);
            break;
        }
        return s;
    }

    static void print(const int *states){
        const int N_CACHE_NUM = 7;
        string s[N_CACHE_NUM];

        for (size_t i = 0; i < N_CACHE_NUM; i++)
        {
            s[i] = stateTostring(states[i]);
        }

        static int num = 0;
        printf("\nNUM[%d]:\n",num++);
        printf("[%s]-[%s]   [%s]-[%s]\n"  , s[0].c_str(), s[1].c_str(), s[2].c_str(), s[3].c_str() );
        printf("  [%s]       [%s]\n"      , s[4].c_str(), s[5].c_str());
        printf("       [%s]\n"            , s[6].c_str());
    }


}
#endif //TLC_TEST_TOOL_H