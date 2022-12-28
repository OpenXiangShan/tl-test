

#include "SAgent.h"

namespace tl_agent{



    void Slave_ScoreBoard::a_write(Channel_A *chan_a){
        Trans *temp = new Trans();
        temp->opcode  = chan_a->opcode;
        temp->param   = chan_a->param;
        temp->source  = chan_a->source;
        temp->address = chan_a->address;
        acquire_q.push_back(temp);
    }

    void Slave_ScoreBoard::c_write(Channel_C *chan_c, uint32_t beat){
        switch(chan_c->opcode){
            case OP_Release:{
                Trans *temp = new Trans();
                temp->state   = S_Finish;
                temp->opcode  = chan_c->opcode;
                temp->param   = chan_c->param;
                temp->source  = chan_c->source;
                temp->address = chan_c->address;
                releaseAck_q.push_back(temp);
                break;
            }
            case OP_ReleaseData:{
                switch(beat){
                    case 1:{
                        Trans *temp = new Trans();
                        temp->state   = S_Wait;
                        temp->opcode  = chan_c->opcode;
                        temp->param   = chan_c->param;
                        temp->source  = chan_c->source;
                        temp->address = chan_c->address;
                        DATA_COPY(temp->data1, chan_c->data);
                        releaseAck_q.push_back(temp);
                        break;
                    }
                    case 2:{
                        uint32_t index = releaseAck_q.size();
                        Trans *temp = releaseAck_q[index - 1];
                        temp->state = S_Finish;
                        DATA_COPY(temp->data2, chan_c->data);
                        break;
                    }
                    default:{
                        assert(0);
                    }
                }
                break;
            }
            case OP_ProbeAck:{                
                if(probeAck_q.empty()) assert(0);

                Trans *temp = NULL;
                for(int i = 0; i < probeAck_q.size(); i++){
                    if(chan_c->address == probeAck_q[i]->address){
                        temp = probeAck_q[i];
                        probeAck_q.erase(probeAck_q.begin() + i);
                        break;
                    }
                }
                if(temp == NULL) assert(0);

                delete temp;
                break;
            }
            case OP_ProbeAckData:{
                if(probeAck_q.empty()) assert(0);

                Trans *temp = NULL;
                switch(beat){
                    case 1:{
                        DATA_COPY(MEM[(chan_c->address >> 6)*2 + 0], chan_c->data);
                        break;
                    }
                    case 2:{
                        DATA_COPY(MEM[(chan_c->address >> 6)*2 + 1], chan_c->data);
                        for(int i = 0; i < probeAck_q.size(); i++){
                            if(chan_c->address == probeAck_q[i]->address){
                                temp = probeAck_q[i];
                                probeAck_q.erase(probeAck_q.begin() + i);
                                break;
                            }
                        }
                        if(temp == NULL) assert(0);

                        delete temp;
                        break;
                    }
                    default:{
                        assert(0);
                        break;
                    }
                }
                break;
            }
            default:{
                assert(0);
            }
        }
    }

    void Slave_ScoreBoard::e_write(Channel_E *chan_e){
        Trans *temp = NULL;
        bool no_find_sink = true;

        for(int i = 0; i < grantAck_q.size(); i++){
            if(chan_e->sink == grantAck_q[i]->sink){
                temp = grantAck_q[i];
                grantAck_q.erase(grantAck_q.begin() + i);
                no_find_sink = false;
                break;
            }
        }
        if(no_find_sink) assert(0);

        delete temp;
    }





void Input_Monitor::monitor_a(Channel_A *chan_a){
    if(chan_a->valid && chan_a->ready){
        scb->a_write(chan_a);
    }
}

void Input_Monitor::monitor_c(Channel_C *chan_c){
    static uint32_t beat = 1;

    if(chan_c->valid && chan_c->ready){
        switch(chan_c->opcode){
            case OP_Release: 
            case OP_ProbeAck:{
                scb->c_write(chan_c, 0);
                break;                    
            }
            case OP_ReleaseData:
            case OP_ProbeAckData:{
                switch(beat){
                    case 1: {
                        scb->c_write(chan_c, beat++);
                        break;
                    }
                    case 2:{
                        scb->c_write(chan_c, beat++);
                        beat = 1;
                        break;
                    }
                    default:{
                        assert(0);
                        break;
                    }
                }
                break;
            }
            default:{
                assert(0);
                break;
            }
        }
    }
}

void Input_Monitor::monitor_e(Channel_E *chan_e){
    if(chan_e->valid && chan_e->ready){
        scb->e_write(chan_e);
    }
}






Trans *Generator::generator_b(){
    if(!scb->genetator_new_tran_b) return NULL;

    uint32_t rand_sel = rand() % 40;
    static uint8_t source = 1;
    if(source == 15) source = 1;

    if(rand_sel == 1){
        Trans *temp = new Trans();
        // generator probe
        temp->opcode   = (rand() % 2 == 0) ? OP_ProbeBlock : OP_ProbePerm;
        temp->param    = PARAM_toN;
        temp->size     = 6;
        temp->source   = source++;
        temp->address  = (paddr_t)rand() << 6;
        temp->mask     = 0xffffffff;
        temp->sink     = 0;
        temp->corrupt  = 0;
        temp->denied   = 0;
        // address check
        bool addr_is_legal1 = true;
        bool addr_is_legal2 = true;
        for(int i = 0; i < scb->grantAck_q.size(); i++){
            if(temp->address == scb->grantAck_q[i]->address){
                addr_is_legal1 = false;
                break;
            }
        }
        for(int i = 0; i < scb->probeAck_q.size(); i++){
            if(temp->address == scb->probeAck_q[i]->address){
                addr_is_legal2 = false;
            }
        }
        
        if(addr_is_legal1 && addr_is_legal2){
            scb->probeAck_q.push_back(temp);
            return temp;
        }

        delete temp;
        return NULL;
    }
    return NULL;
}






Trans *Generator::generator_d(){
    if(!scb->genetator_new_tran_d) return NULL;

    uint32_t rand_sel = rand() % 40;
    static uint32_t sink = 1;
    if(sink == 15) sink = 1;

    if(rand_sel == 1){
        if(scb->acquire_q.empty()) return NULL;

        Trans *tran_acquire = scb->acquire_q[0];
        tran_acquire->sink = sink++;
        scb->acquire_q.erase(scb->acquire_q.begin());
        scb->grantAck_q.push_back(tran_acquire);

        Trans *temp = new Trans();
        temp->has_data = (tran_acquire->opcode == OP_AcquireBlock);
        temp->opcode   = (tran_acquire->opcode == OP_AcquireBlock) ? OP_GrantData :
                         (tran_acquire->opcode == OP_AcquirePerm)  ? OP_Grant : 
                         -1;
        temp->param    = (tran_acquire->param == PARAM_NtoB) ? PARAM_toB :
                         (tran_acquire->param == PARAM_NtoT) ? PARAM_toT :
                         (tran_acquire->param == PARAM_BtoT) ? PARAM_toT :
                         -1;
        temp->size     = 6;
        temp->source   = tran_acquire->source;
        temp->address  = tran_acquire->address;
        temp->mask     = 0xffffffff;
        temp->sink     = tran_acquire->sink;
        temp->corrupt  = 0;
        temp->denied   = 0;
        DATA_COPY(temp->data1,scb->MEM[(temp->address >> 6)*2 + 0]);
        DATA_COPY(temp->data2,scb->MEM[(temp->address >> 6)*2 + 1]);

        return temp;   
    }
    else if(rand_sel == 2){
        if(scb->releaseAck_q.empty()) return NULL;

        Trans *temp = scb->releaseAck_q[0];
        if(temp->state != S_Finish) return NULL;
        scb->releaseAck_q.erase(scb->releaseAck_q.begin());
        temp->has_data = (temp->opcode == OP_ReleaseData);
        temp->opcode   = OP_ReleaseAck;
        temp->param    = 0;
        temp->size     = 6;
        temp->source   = temp->source;
        temp->address  = temp->address;
        temp->mask     = 0xffffffff;
        temp->sink     = 0;
        temp->corrupt  = 0;
        temp->denied   = 0;
        if(temp->has_data){
            DATA_COPY(scb->MEM[(temp->address >> 6)*2 + 0], temp->data1);
            DATA_COPY(scb->MEM[(temp->address >> 6)*2 + 1], temp->data2);
        }
        
        return temp;
    }

    return NULL;
}





void Driver::driver_b(Trans *tran){
    static bool tran_finish = true;
    static Trans *temp = NULL;

    if(tran_finish){
        chan_b->valid = false;
        if(tran == NULL) return;

        temp = tran;
        chan_b->valid   = true;
        chan_b->opcode  = tran->opcode;
        chan_b->param   = tran->param;
        chan_b->size    = tran->size;
        chan_b->source  = tran->source;
        chan_b->address = tran->address;
        chan_b->mask    = tran->mask;
        chan_b->corrupt = tran->corrupt;
        tran_finish = false;
    }

    if(chan_b->valid && chan_b->ready){
        tran_finish = true;
    }
    scb->genetator_new_tran_b = tran_finish;
}







void Driver::driver_d(Trans *tran){
    static bool tran_finish = true;
    static uint32_t beat = 1;
    static Trans *temp = NULL;

    if(tran_finish){
        chan_d->valid = false;
        if(tran == NULL) return;

        temp = tran;
        chan_d->valid   = true;
        chan_d->opcode  = temp->opcode;
        chan_d->param   = temp->param;
        chan_d->size    = temp->size;
        chan_d->source  = temp->source;
        chan_d->sink    = temp->sink;
        chan_d->denied  = temp->denied;
        chan_d->corrupt = temp->corrupt;
        DATA_COPY(chan_d->data, temp->data1);
        tran_finish = false;
    }

    if(beat == 2){
        DATA_COPY(chan_d->data, temp->data2);
    }

    if(chan_d->valid && chan_d->ready){
        switch(chan_d->opcode){
            case OP_Grant:
            case OP_ReleaseAck:{
                tran_finish = true;
                delete temp;
                break;
            }
            case OP_GrantData:{
                if(beat == 1){
                    tran_finish = false;
                    beat++;
                }
                else if(beat == 2){
                    tran_finish = true;
                    beat = 1;
                    delete temp;
                }
                break;
            }
            default:{
                assert(0);
                break;
            }
        }
    }

    scb->genetator_new_tran_d = tran_finish;
}


void Driver::driver_ready(){
    chan_a->ready = true;
    chan_c->ready = true;
    chan_e->ready = true;
}


}