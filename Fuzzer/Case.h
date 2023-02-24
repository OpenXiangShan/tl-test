//
// Created by dxy on 2023/2/13.
//

/*
#Run case mode('-r') :

> cd build
> touch test.txt

Write Case according to the following format

Format:				
case	cycle	agent	agentid	link	operation	opcode	param	paramcode	address	uesr	
Example:					
0	100	    D0	    0	0	AB	6	NtoT	1	1	0						
0	9000	D1	    2	1	AP	7	NtoT	1	1	0						
0	9000	PTW0	4	2	GET	4	0	    0	1	0						
0	9000	DMA	    6	3	PFD	0	0	    0	1	0						
0	9000	I1	    3	4	AB	6	NtoB	0	1	0						
0	9500	DMA	    6	5	PPD	1	0	    0	1	0						
0	9500	D1	    2	6	AP	7	NtoT	1	1	0						
0	10000	RESET	0	-1	RE	16	0	    0	0	0						
0	10050	D1	    2	7	RD	8	TtoN	1	0	0


Description:
opcode:
0	1	2	3	4	5	6	7	8	16				
PFD	PPD	AD	LD	GET	HINT	AB	AP	RD	RE				
param:													
0	1	2	0	1	2		
NtoB	NtoT	BtoT	TtoB	TtoN	BtoN					
agent:
D0	I0	D1	I1	PTW0	PTW1	DMA
0	1	2	3	4	5	6
address:
Refresh Repeat addr
1   0   else

*/


#ifndef TLC_TEST_CASE_H
#define TLC_TEST_CASE_H

#include "../Utils/Common.h"
#include "../TLAgent/Port.h"
#include <map>
#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<sstream>

namespace testcase {

    enum{
        n_case = 0,
        n_cycle,
        n_id = 3,
        n_link,
        n_op = 6,
        n_param = 8,
        n_addr,
        // n_data,
        n_user,
        n_last,//11
    };

    enum{
        N_TESTCASE_AGENT = 7,//L1*4+PTW*2+DMA
    };

    enum{
        reset_opcode = 16
    };

    class Mes_Entry {
    public:
        int link;
        int id;//agent_id
        int test_case;
        paddr_t addr;
        uint8_t opcode;
        uint8_t param;
        uint8_t user;
    };

    //cycle,Entry
    class Message {
    protected:
        std::map<uint64_t, Mes_Entry> mapping;//<addr, Mes>
    public:
        Message();
        ~Message();
        void clear(void);
        void update(const uint64_t& key,const Mes_Entry& mes);
        bool haskey(const uint64_t& key);
        Mes_Entry query(const uint64_t& key);//return mapping[key]
    };

    class read_file{
    protected:
        const long arr_size = 10000;
        
    public:
        read_file() = default;
        void read_test(Message agent_message[N_TESTCASE_AGENT]);
    };

}

#endif //TLC_TEST_CASE_H
