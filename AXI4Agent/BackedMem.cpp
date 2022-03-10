/***************************************************************************************
 * Copyright (c) 2020-2021 Institute of Computing Technology, Chinese Academy of
 *Sciences Copyright (c) 2020-2021 Peng Cheng Laboratory
 *
 * XiangShan is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "BackedMem.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/mman.h>

static uint64_t *ram;

// in-flight AXI requests
MemReqMeta inflt_r_resp_meta;
MemReqMeta inflt_w_req_meta;
MemReqMeta inflt_w_resp_meta;

uint64_t read_resp_counter = 0;

void init_ram_backed_store() {
  printf("Using simulated %luMB RAM\n", EMU_RAM_SIZE / (1024 * 1024));
  ram = (uint64_t *)mmap(NULL, EMU_RAM_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
  if (ram == (uint64_t *)MAP_FAILED) {
    printf("Cound not mmap 0x%lx bytes\n", EMU_RAM_SIZE);
    assert(false);
  }
}

void axi_setup_read_meta(const axi_ar_channel &ar, MemReqMeta *meta) {
  uint64_t address = *ar.addr % EMU_RAM_SIZE;
  uint64_t beatsize = 1 << *ar.size;
  uint8_t beatlen = *ar.len + 1;
  uint64_t transaction_size = beatsize * beatlen;
  assert((transaction_size % sizeof(uint64_t)) == 0);
  // axi burst FIXED
  if (ar.burst == 0x0) {
    std::cout << "axi burst FIXED not supported!" << std::endl;
    assert(0);
  } else if (*ar.burst == 1) { // axi burst INCR
    assert(transaction_size / sizeof(uint64_t) <= MAX_AXI_DATA_LEN);
    for (int i = 0; i < transaction_size / sizeof(uint64_t); i++) {
      meta->data[i] = ram[address / sizeof(uint64_t)];
      address += sizeof(uint64_t);
    }
  } else if (*ar.burst == 2) { // axi burst WRAP
    uint64_t low = (address / transaction_size) * transaction_size;
    uint64_t high = low + transaction_size;
    assert(transaction_size / sizeof(uint64_t) <= MAX_AXI_DATA_LEN);
    for (int i = 0; i < transaction_size / sizeof(uint64_t); i++) {
      if (address == high) {
        address = low;
      }
      meta->data[i] = ram[address / sizeof(uint64_t)];
      address += sizeof(uint64_t);
    }
  } else {
    std::cout << "reserved arburst!" << std::endl;
    assert(0);
  }
  meta->len = beatlen;
  meta->size = beatsize;
  meta->offset = 0;
  meta->id = *ar.id;
  meta->valid = true;
}

void axi4_mem_rising(const axi_channel &axi) {
  // printf("Rising\n");

  if (axi_check_rdata_fire(axi)) {
    printf("Read resp data at %lx, offset %u fired\n", inflt_r_resp_meta.addr, inflt_r_resp_meta.offset);
    assert(inflt_r_resp_meta.valid);
    inflt_r_resp_meta.offset++;
    if (inflt_r_resp_meta.finished()) { // data resp finished
      inflt_r_resp_meta.valid = false;
      printf("Read resp data at %lx finished, served %lu requests in total\n",
             inflt_r_resp_meta.addr, ++read_resp_counter);
    }
  }

  if (axi_check_raddr_fire(axi)) {
    // printf("Firing read addr 0x%lx\n", *axi.ar.addr);
    axi_setup_read_meta(axi.ar, &inflt_r_resp_meta);
    printf("Fired read addr 0x%lx, id:%u\n", *axi.ar.addr, *axi.ar.id);
  }

  if (axi_check_wack_fire(axi)) {
    assert(inflt_w_resp_meta.valid);
    uint64_t waddr = inflt_w_resp_meta.addr;
    void * start_addr = ram + (waddr / sizeof(uint64_t));
    memcpy(start_addr, inflt_w_resp_meta.data,
           inflt_w_resp_meta.len * inflt_w_resp_meta.size);
    inflt_w_resp_meta.valid = false;
  }

  if (axi_check_waddr_fire(axi)) {
    assert(!inflt_w_req_meta.valid);
    inflt_w_req_meta.valid = true;
    inflt_w_req_meta.len = *axi.aw.len + 1;
    inflt_w_req_meta.size = *axi.aw.size;
    inflt_w_req_meta.offset = 0;
    inflt_w_req_meta.id = *axi.aw.id;
    inflt_w_req_meta.addr = *axi.aw.addr;
  }

  if (axi_check_wdata_fire(axi)) {
    assert(inflt_w_req_meta.valid);
    void *data_start = inflt_w_req_meta.compute_data_start();
    uint64_t waddr = *axi.aw.addr % EMU_RAM_SIZE; 
    const void *src_addr = ram + (waddr + inflt_w_req_meta.offset * inflt_w_req_meta.size) / sizeof(uint64_t);
    // why source addr is in ram? To support mask
    axi_get_wdata(axi, data_start, src_addr, inflt_w_req_meta.size);
    inflt_w_req_meta.offset++;
  }
  if (inflt_w_req_meta.valid) {
    if (inflt_w_req_meta.finished()) {
      inflt_w_req_meta.valid = false;
    }
  }
}

void axi4_mem_falling(const axi_channel &axi) {
  // printf("Falling\n");

  *axi.aw.ready = 0;
  *axi.w.ready = 0;
  *axi.b.valid = 0;
  *axi.ar.ready = 0;
  *axi.r.valid = 0;

  if (!inflt_r_resp_meta.valid) {
    // there should not be multiple pending requests
  }
  if (inflt_r_resp_meta.valid) { // put data from meta to axi bus
    printf("Putting read data to AXI, addr: 0x%lx\n", inflt_r_resp_meta.addr);
    void *data_start = inflt_r_resp_meta.compute_data_start();
    axi_put_rdata(axi, data_start, (size_t)inflt_r_resp_meta.size,
                  inflt_r_resp_meta.is_last_chunk(), inflt_r_resp_meta.id);
  }
  // printf("reach 0\n");
  axi_addr_t raddr;
  if (axi_get_raddr(axi, raddr) && !inflt_r_resp_meta.valid) {
    printf("Accept read addr 0x%lx\n", raddr);
    axi_accept_raddr(axi);
    inflt_r_resp_meta.addr = raddr;
  }
  // printf("reach 1\n");

  axi_addr_t waddr;
  if (!inflt_w_req_meta.valid && axi_get_waddr(axi, waddr)) {
    axi_accept_waddr(axi);
    axi_accept_wdata(axi);
  }
  // printf("reach 2\n");

  if (inflt_w_req_meta.valid && !inflt_w_req_meta.finished()) {
    axi_accept_wdata(axi);
  }

  if (!inflt_w_resp_meta.valid) {
    // there should not be multiple pending requests
  }
  if (inflt_w_resp_meta.valid) {
    axi_put_wack(axi, inflt_w_resp_meta.id);
  }
  // printf("reach 3\n");
}