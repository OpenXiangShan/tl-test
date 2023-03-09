#include "sequencer.h"

namespace sequencer {

std::shared_ptr<uint8_t[]> gen_put_data(std::shared_ptr<uint8_t[]>putdata) {
    for (int i = 0; i < DATASIZE; i++) {
        putdata[i] = (uint8_t)rand();
    }
    return putdata;
}

std::shared_ptr<uint8_t[]> gen_putpartial_data(std::shared_ptr<uint8_t[]>putdata) {
    for (int i = 0; i < DATASIZE / 2; i++) {
        putdata[i] = (uint8_t)rand();
    }
    for (int i = DATASIZE / 2; i < DATASIZE; i++) {
        putdata[i] = putdata[i - DATASIZE / 2];
    }
    return putdata;
}

paddr_t gen_rand_addr() {
    return (rand() % 0x8) * (rand() % 0x80) * 0x40 + DRAM_OFFSET; // Tag + Set + Offset
}

tl_base_agent::TLCTransaction Sequencer::random_test_fullsys(uint8_t tr_type, bool do_alias, uint8_t bus_type,
                                                                std::shared_ptr<fake_ptw::FakePTW> *ptw, 
                                                                std::shared_ptr<fake_ptw::FakePTW> *dma) {
    using namespace tl_base_agent;
    TLCTransaction tr;
    paddr_t addr = 0;
    int alias = 0;
    uint8_t param = 0;
    std::shared_ptr<uint8_t[]>putdata(new uint8_t[DATASIZE]);

    addr = gen_rand_addr();
    tr.opcode = INVALID_REQ;
    tr.addr = addr;

    switch (tr_type) {
        case TLC:
            // probe all ul-agents
            for (int j = 0; j < NR_PTWAGT; j++) {
                if(ptw[j]->local_probe(addr)) {
                    goto invalid_req;
                }
            }
            for (int j = 0; j < NR_DMAAGT; j++) {
                if(dma[j]->local_probe(addr)) {
                    goto invalid_req;
                }
            }
            alias = (do_alias) ? (rand() % 4) : 0;
            param = bus_type == ICACHE_BUS_TYPE? tl_agent::NtoB:tl_agent::NtoT;
            tr.alias = alias;
            tr.param = param;
            if (rand() % 2) {
                if (rand() % 3) {
                    if (rand() % 2) {
                        tr.opcode = ACQUIRE_BLOCK;
                    } else {
                        // TODO: ??
                        tr.opcode = ACQUIRE_BLOCK;
                    }
                }else {
                    if(bus_type == DCACHE_BUS_TYPE) {
                        tr.opcode = ACQUIRE_PERM;
                    }
                }
            } else {
                tr.opcode = RELEASE_DATA_AUTO;
            }
            break;
        case TLUL:
            if (rand() % 2 || bus_type == PTW_BUS_TYPE) { // Get
                tr.opcode = GET_AUTO;
            } else {
                if (rand() % 2) { // PutFullData
                    tr.opcode = PUT_FULL_DATA;
                    tr.data = gen_put_data(putdata);
                } else { // PutPartialData
                    uint32_t mask_raw = 0;
                    uint8_t offset;
                    for (int i = 0; i < 4; i++) {
                        mask_raw = (mask_raw << 8) + rand() % 0xFF;
                    }
                    switch (rand() % 3) {
                        case 0:
                            offset = (rand() % 4) * 8;
                            tr.opcode = PUT_PARTIAL_DATA;
                            tr.offset = offset;
                            tr.size = 3;
                            tr.mask = mask_raw & (0x000000FF << offset);
                            tr.data = gen_putpartial_data(putdata);
                            break;
                        case 1:
                            offset = (rand() % 2) * 16;
                            tr.opcode = PUT_PARTIAL_DATA;
                            tr.offset = offset;
                            tr.size = 4;
                            tr.mask = mask_raw & (0x0000FFFF << offset);
                            tr.data = gen_putpartial_data(putdata);
                            break;
                        case 2:
                            tr.opcode = PUT_PARTIAL_DATA;
                            tr.offset = 0;
                            tr.size = 5;
                            tr.mask = mask_raw & 0xFFFFFFFF;
                            tr.data = gen_putpartial_data(putdata);
                            break;
                    }
                }
            }
            break;
        default:
            break;
    }
invalid_req:
    return tr;
}

tl_base_agent::TLCTransaction Sequencer::case_test(uint8_t tr_type, int id) {
    using namespace tl_base_agent;
    TLCTransaction tr;
    int alias = 0;
    uint8_t param = 0;
    std::shared_ptr<uint8_t[]>putdata(new uint8_t[DATASIZE]);
    tr.opcode = INVALID_REQ;

    if(case_mes[id].haskey(*cycles) == true)
    {
        std::shared_ptr<uint8_t[]>putdata(new uint8_t[DATASIZE]);
        // std::cout << "agent:" << id << " haskey" << std::endl;
        testcase::Mes_Entry mes = case_mes[id].query(*cycles);
        
        // generate address
        static paddr_t addr = 0x80000000;
        if(mes.addr == 0)       addr = addr;
        else if(mes.addr == 1)  addr = gen_rand_addr();
        else                    addr = mes.addr;
        tr.addr = addr;
        
        switch (tr_type) {
            case TLC:
                tr.param = mes.param;
                tr.alias = mes.user;
                switch(mes.opcode){
                    case tl_agent::AcquireBlock:
                        tr.opcode = ACQUIRE_BLOCK;
                        break;
                    case tl_agent::AcquirePerm:
                        tr.opcode = ACQUIRE_PERM;
                        break;
                    case (tl_agent::ReleaseData+1):// 因为releasdata与acquireperm的param重复，+1以视为区分
                        for (int i = 0; i < DATASIZE; i++) {
                            putdata[i] = (uint8_t)rand();
                        }
                        tr.opcode = RELEASE_DATA;
                        tr.data = putdata;
                        break; 
                    case testcase::reset_opcode: 
                        break;
                    default:
                        printf("caseTest error\n");
                        break;
                }
            case TLUL:
                switch(mes.opcode) {
                    case tl_agent::Get:
                        tr.opcode = GET_AUTO;
                        break;
                    case tl_agent::PutFullData:
                        tr.opcode = PUT_FULL_DATA;
                        tr.data = gen_put_data(putdata);
                        break;
                    case tl_agent::PutPartialData:
                        uint32_t mask_raw = 0;
                        uint8_t offset;
                        for (int i = 0; i < 4; i++) {
                            mask_raw = (mask_raw << 8) + rand() % 0xFF;
                            tr.data = gen_putpartial_data(putdata);
                        }
                        switch (rand() % 3) {
                            case 0:
                                offset = (rand() % 4) * 8;
                                tr.opcode = PUT_PARTIAL_DATA;
                                tr.offset = offset;
                                tr.size = 3;
                                tr.mask = mask_raw & (0x000000FF << offset);
                                tr.data = gen_putpartial_data(putdata);
                                break;
                            case 1:
                                offset = (rand() % 2) * 16;
                                tr.opcode = PUT_PARTIAL_DATA;
                                tr.offset = offset;
                                tr.size = 4;
                                tr.mask = mask_raw & (0x0000FFFF << offset);
                                tr.data = gen_putpartial_data(putdata);
                                break;
                            case 2:
                                tr.opcode = PUT_PARTIAL_DATA;
                                tr.offset = 0;
                                tr.size = 5;
                                tr.mask = mask_raw & 0xFFFFFFFF;
                                tr.data = gen_putpartial_data(putdata);
                                break;
                        }
                        break;
                }
        }
    }

invalid_req:
    return tr;
}


}
