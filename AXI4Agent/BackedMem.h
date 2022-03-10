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

#ifndef __BackedMem_H
#define __BackedMem_H

#include "AXI4Agent.h"

#define EMU_RAM_SIZE 8UL * 1024UL * 1024UL * 1024UL

void axi_read_data(const axi_ar_channel &ar);

struct MemReqMeta {
  bool valid{};
  uint64_t addr{};
  uint8_t len{};
  uint8_t size{};
  uint8_t offset{};
  uint8_t id{};
  uint8_t data[MAX_AXI_DATA_LEN];

  void *compute_data_start () {
    return data + offset * size / sizeof(uint64_t);
  }

  bool is_last_chunk() {
    return offset == len - 1;
  }
  bool finished() {
    return offset == len;
  }
};

void axi4_mem_rising(const axi_channel &axi);
void axi4_mem_falling(const axi_channel &axi);

void init_ram_backed_store();

#endif // __BackedMem_H