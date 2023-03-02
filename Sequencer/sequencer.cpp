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

tl_base_agent::TLCTransaction Sequencer::random_test(uint8_t tr_type, bool do_alias, uint8_t bus_type) {
    using namespace tl_base_agent;
    TLCTransaction tr;
    paddr_t addr;
    int alias;
    uint8_t param;
    std::shared_ptr<uint8_t[]>putdata(new uint8_t[DATASIZE]);

    addr = (rand() % 0x8) * (rand() % 0x80) * 0x40 + DRAM_OFFSET; // Tag + Set + Offset
    tr.opcode = INVALID_REQ;
    tr.addr = addr;

    switch (tr_type) {
        case TLC:
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
    return tr;
}

}