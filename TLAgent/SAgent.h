#ifndef TLC_TEST_SAGENT_H
#define TLC_TEST_SAGENT_H

#include "../Utils/Common.h"
#include<iostream>
#include<cstdio>

namespace tl_agent{


using namespace std;


#define DATA_COPY(dest, src) { \
  for(int Index_i = 0; Index_i < 8; Index_i++){ \
    dest[Index_i] = src[Index_i]; \
  } \
}




enum Cap    { PARAM_toT  = 0, PARAM_toB,  PARAM_toN  };
enum Grow   { PARAM_NtoB = 0, PARAM_NtoT, PARAM_BtoT };
enum Prune  { PARAM_TtoB = 0, PARAM_TtoN, PARAM_BtoN };
enum Report { PARAM_TtoT = 3, PARAM_BtoB, PARAM_NtoN };


enum {OP_Grant      = 4, OP_GrantData,    OP_AcquireBlock, OP_AcquirePerm };
enum {OP_ProbeAck   = 4, OP_ProbeAckData, OP_ProbeBlock,   OP_ProbePerm   };
enum {OP_Release    = 6, OP_ReleaseData                                   };
enum {OP_ReleaseAck = 6                                                   };

enum {S_Wait = 1, S_Finish};

class Channel_A{
  public:
    bool      valid;
    bool      ready;
    uint8_t   opcode;
    uint8_t   param;
    uint8_t   size;
    uint8_t   source;
    paddr_t   address;
    uint32_t  mask;
    bool      corrupt;
    uint32_t  data[8];
};

class Channel_B{
  public:
    bool      valid;
    bool      ready;
    uint8_t   opcode;
    uint8_t   param;
    uint8_t   size;
    uint8_t   source;
    paddr_t  address;
    uint32_t  mask;
    bool      corrupt;
    uint32_t  data[8];
};

class Channel_C{
  public:
    bool      valid;
    bool      ready;
    uint8_t   opcode;
    uint8_t   param;
    uint8_t   size;
    uint8_t   source;
    paddr_t  address;
    bool      corrupt;
    uint32_t  data[8];
};

class Channel_D{
  public:
    bool      valid;
    bool      ready;
    uint8_t   opcode;
    uint8_t   param;
    uint8_t   size;
    uint8_t   source;
    uint8_t   sink;
    bool      denied;
    bool      corrupt;
    uint32_t  data[8];
};

class Channel_E{
  public:
    bool      valid;
    bool      ready;
    uint8_t   sink;
};


class Trans{
    public:
        bool      has_data;
        uint8_t   state;
        uint8_t   opcode;
        uint8_t   param;
        uint8_t   size;
        uint8_t   source;
        paddr_t   address;
        uint32_t  mask;
        uint8_t   sink;
        bool      corrupt;
        bool      denied;
        uint32_t  data1[8];
        uint32_t  data2[8];
};





class Slave_ScoreBoard{
    public:
        uint32_t MEM[16*1024][8];
        vector<Trans*> acquire_q;
        vector<Trans*> grantAck_q;

        vector<Trans*> releaseAck_q;

        vector<Trans*> probeAck_q;
        vector<Trans*> lib_probe;

        bool genetator_new_tran_b;
        bool genetator_new_tran_d;

    public:
        Slave_ScoreBoard(){
            genetator_new_tran_b = true;
            genetator_new_tran_d = true;
            for(int i = 0; i < 16*1024; i++){
                for(int j = 0; j < 8; j++){
                    MEM[i][j] = 0;
                }
            }
        }

    void a_write(Channel_A *chan_a);
    void c_write(Channel_C *chan_c, uint32_t beat);
    void e_write(Channel_E *chan_e);
};


class Input_Monitor{
    private:
        Slave_ScoreBoard *scb;

    public:
        Input_Monitor(Slave_ScoreBoard *scb){
            this->scb = scb;
        }

        void monitor_a(Channel_A *chan_a);
        void monitor_c(Channel_C *chan_c);
        void monitor_e(Channel_E *chan_e);
};





class Generator{
  private:
    Slave_ScoreBoard *scb;

  public:
    Generator(Slave_ScoreBoard *scb){
      this->scb = scb;
    }

    Trans *generator_b();
    Trans *generator_d();
};


class Driver{
  private:
    Slave_ScoreBoard *scb;
    Channel_A *chan_a;
    Channel_B *chan_b;
    Channel_C *chan_c;
    Channel_D *chan_d;
    Channel_E *chan_e;


  public:
    Driver(Slave_ScoreBoard *scb, Channel_A *chan_a, Channel_B *chan_b, Channel_C *chan_c, Channel_D *chan_d, Channel_E *chan_e){
      this->scb    = scb;
      this->chan_a = chan_a;
      this->chan_b = chan_b;
      this->chan_c = chan_c;
      this->chan_d = chan_d;
      this->chan_e = chan_e;
    }

    void driver_d(Trans *tran);
    void driver_ready();
};



}




#endif //TLC_TEST_SAGENT_H

