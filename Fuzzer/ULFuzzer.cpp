//
// Created by wkf on 2021/10/29.
//

#include "Fuzzer.h"

ULFuzzer::ULFuzzer(std::shared_ptr<tl_agent::ULAgent> ulAgent) { this->ulAgent = ulAgent; }

std::shared_ptr<uint8_t[]>genPutData(std::shared_ptr<uint8_t[]>putdata) {
  for (int i = 0; i < DATASIZE; i++) {
    putdata[i] = (uint8_t)rand();
  }
  return putdata;
}

std::shared_ptr<uint8_t[]>genPutPartialData(std::shared_ptr<uint8_t[]>putdata) {
  for (int i = 0; i < DATASIZE / 2; i++) {
    putdata[i] = (uint8_t)rand();
  }
  for (int i = DATASIZE / 2; i < DATASIZE; i++) {
    putdata[i] = putdata[i - DATASIZE / 2];
  }
  return putdata;
}

// void ULFuzzer::randomTest(std::shared_ptr<tl_agent::BaseAgent> *agent) {
//   // if (rand() % 10) return;
//   // address generation
//   bool sent = false;
//   paddr_t addr;
//   bool flag;
//   std::shared_ptr<uint8_t[]>putdata(new uint8_t[DATASIZE]);
//   for (int i = 0; i < 10; i++) {
//     addr = (rand() % 0x8) * (rand() % 0x80) * 0x40 + DRAM_OFFSET; // Tag + Set + Offset
//     flag = true;
//     // probe all agents including itself
//     for (int j = 0; j < NR_CAGENTS + NR_ULAGENTS; j++) {
//       if (agent[j]->local_probe(addr)) {
//         flag = false;
//         break;
//       }
//     }
//     if (flag)
//       break;
//   }
//   if (!flag)
//     return;

//   if (rand() % 2 || this->ulAgent->bus_type == PTW_BUS_TYPE) { // Get
//     ulAgent->do_getAuto(addr);
//   } else {
//     if (rand() % 2) { // PutFullData
//       sent = ulAgent->do_putfulldata(addr, genPutData(putdata));
//     } else { // PutPartialData
//       uint32_t mask_raw = 0;
//       int offset;
//       for (int i = 0; i < 4; i++) {
//         mask_raw = (mask_raw << 8) + rand() % 0xFF;
//       }
//       switch (rand() % 3) {
//       case 0:
//         offset = (rand() % 4) * 8;
//         sent = ulAgent->do_putpartialdata(addr, offset, 3,
//                                           mask_raw & (0x000000FF << offset),
//                                           genPutPartialData(putdata));
//         break;
//       case 1:
//         offset = (rand() % 2) * 16;
//         sent = ulAgent->do_putpartialdata(addr, offset, 4,
//                                           mask_raw & (0x0000FFFF << offset),
//                                           genPutPartialData(putdata));
//         break;
//       case 2:
//         sent = ulAgent->do_putpartialdata(addr, 0, 5, mask_raw & 0xFFFFFFFF,
//                                           genPutPartialData(putdata));
//         break;
//       }
//     }
//   }
// }

void ULFuzzer::randomTest(std::shared_ptr<tl_agent::BaseAgent> *agent, std::shared_ptr<fake_l1::FakeL1> *l1) {
  // // if (rand() % 10) return;
  // // address generation
  // bool sent = false;
  // paddr_t addr;
  // bool flag;
  // std::shared_ptr<uint8_t[]>putdata(new uint8_t[DATASIZE]);
  // for (int i = 0; i < 10; i++) {
  //   addr = (rand() % 0x8) * (rand() % 0x80) * 0x40 + DRAM_OFFSET; // Tag + Set + Offset
  //   flag = true;
  //   // probe all agents including itself
  //   for(int k = 0; k < NR_CAGENTS; k++) {
  //     if(l1[k]->local_probe(addr)){
  //       flag = false;
  //       break;
  //     }
  //   }
  //   if(flag) {
  //     for (int j = NR_CAGENTS; j < NR_CAGENTS + NR_ULAGENTS; j++) {
  //       if (agent[j]->local_probe(addr)) {
  //         flag = false;
  //         break;
  //       }
  //     }
  //   }
  //   if (flag)
  //     break;
  // }
  // if (!flag)
  //   return;

  // if (rand() % 2 || this->ulAgent->bus_type == PTW_BUS_TYPE) { // Get
  //   ulAgent->do_getAuto(addr);
  // } else {
  //   if (rand() % 2) { // PutFullData
  //     sent = ulAgent->do_putfulldata(addr, genPutData(putdata));
  //   } else { // PutPartialData
  //     uint32_t mask_raw = 0;
  //     int offset;
  //     for (int i = 0; i < 4; i++) {
  //       mask_raw = (mask_raw << 8) + rand() % 0xFF;
  //     }
  //     switch (rand() % 3) {
  //     case 0:
  //       offset = (rand() % 4) * 8;
  //       sent = ulAgent->do_putpartialdata(addr, offset, 3,
  //                                         mask_raw & (0x000000FF << offset),
  //                                         genPutPartialData(putdata));
  //       break;
  //     case 1:
  //       offset = (rand() % 2) * 16;
  //       sent = ulAgent->do_putpartialdata(addr, offset, 4,
  //                                         mask_raw & (0x0000FFFF << offset),
  //                                         genPutPartialData(putdata));
  //       break;
  //     case 2:
  //       sent = ulAgent->do_putpartialdata(addr, 0, 5, mask_raw & 0xFFFFFFFF,
  //                                         genPutPartialData(putdata));
  //       break;
  //     }
  //   }
  // }
}

tl_base_agent::TLCTransaction randomTest3(std::shared_ptr<fake_ptw::FakePTW> *ptw, std::shared_ptr<fake_ptw::FakePTW> *dma,std::shared_ptr<fake_l1::FakeL1> *l1, int bus_type) {
  // if (rand() % 10) return;
  // address generation
  using namespace tl_base_agent;
  TLCTransaction tr;
  bool sent = false;
  paddr_t addr;
  bool flag = true;
  tr.opcode = INVALID_REQ;
  std::shared_ptr<uint8_t[]>putdata(new uint8_t[DATASIZE]);
  // for (int i = 0; i < 10; i++) {
    addr = (rand() % 0x8) * (rand() % 0x80) * 0x40 + DRAM_OFFSET; // Tag + Set + Offset
  //   flag = true;
  //   // probe all agents including itself
  //   for(int j = 0; j < NR_CAGENTS; j++) {
  //     if(l1[j]->local_probe(addr)){
  //       flag = false;
  //       goto invalid_req;
  //       // break;
  //     }
  //   }
  //   for (int j = 0; j < NR_PTWAGT; j++) {
  //     if(ptw[j]->local_probe(addr)) {
  //       flag = false;
  //       goto invalid_req;
  //       // break;
  //     }
  //   }
  //   for (int j = 0; j < NR_DMAAGT; j++) {
  //     if(dma[j]->local_probe(addr)) {
  //       flag = false;
  //       goto invalid_req;
  //       // break;
  //     }
  //   }
  //   if (flag)
  //     break;
  // }

invalid_req:
  if (!flag)
    return tr;

  tr.addr = addr;

  if (rand() % 2 || bus_type == PTW_BUS_TYPE) { // Get
    // ulAgent->do_getAuto(addr);
    tr.opcode = GET_AUTO;
  } else {
    if (rand() % 2) { // PutFullData
      // sent = ulAgent->do_putfulldata(addr, genPutData(putdata));
      tr.opcode = PUT_FULL_DATA;
      tr.data = genPutData(putdata);
    } else { // PutPartialData
      uint32_t mask_raw = 0;
      int offset;
      for (int i = 0; i < 4; i++) {
        mask_raw = (mask_raw << 8) + rand() % 0xFF;
      }
      switch (rand() % 3) {
      case 0:
        offset = (rand() % 4) * 8;
        // sent = ulAgent->do_putpartialdata(addr, offset, 3,
        //                                   mask_raw & (0x000000FF << offset),
        //                                   genPutPartialData(putdata));
        tr.opcode = PUT_PARTIAL_DATA;
        tr.offset = offset;
        tr.size = 3;
        tr.mask = mask_raw & (0x000000FF << offset);
        tr.data = genPutPartialData(putdata);
        break;
      case 1:
        offset = (rand() % 2) * 16;
        // sent = ulAgent->do_putpartialdata(addr, offset, 4,
        //                                   mask_raw & (0x0000FFFF << offset),
        //                                   genPutPartialData(putdata));
        tr.opcode = PUT_PARTIAL_DATA;
        tr.offset = offset;
        tr.size = 4;
        tr.mask = mask_raw & (0x0000FFFF << offset);
        tr.data = genPutPartialData(putdata);
        break;
      case 2:
        // sent = ulAgent->do_putpartialdata(addr, 0, 5, mask_raw & 0xFFFFFFFF,
        //                                   genPutPartialData(putdata));
        tr.opcode = PUT_PARTIAL_DATA;
        tr.offset = 0;
        tr.size = 5;
        tr.mask = mask_raw & 0xFFFFFFFF;
        tr.data = genPutPartialData(putdata);
        break;
      }
    }
  }
  return tr;
}

void ULFuzzer::caseTest(int id) {
    // if(case_mes[id].haskey(*cycles) == true)
    // {
    //     std::shared_ptr<uint8_t[]>putdata(new uint8_t[DATASIZE]);
    //     // std::cout << "agent:" << id << " haskey" << std::endl;
    //     testcase::Mes_Entry mes = case_mes[id].query(*cycles);
        
    //     //genAddr
    //     static paddr_t addr = 0x80000000;
    //     if(mes.addr == 0)
    //       addr = addr;
    //     else if(mes.addr == 1)
    //       addr = genAddr();
    //     else
    //       addr = mes.addr;

    //     switch(mes.opcode){
    //         case tl_agent::Get :
    //             ulAgent->do_getAuto(addr);
    //             break;
    //         case tl_agent::PutFullData :
    //             ulAgent->do_putfulldata(addr, genPutData(putdata));
    //             break;
    //             //TODO : offset could be set
    //         case tl_agent::PutPartialData :
    //             uint32_t mask_raw = 0;
    //             int offset;
    //             for (int i = 0; i < 4; i++) {
    //               mask_raw = (mask_raw << 8) + rand() % 0xFF;
    //             }
    //             switch (rand() % 3) {
    //             case 0:
    //               offset = (rand() % 4) * 8;
    //               ulAgent->do_putpartialdata(addr, offset, 3,
    //                                                 mask_raw & (0x000000FF << offset),
    //                                                 genPutPartialData(putdata));
    //               break;
    //             case 1:
    //               offset = (rand() % 2) * 16;
    //               ulAgent->do_putpartialdata(addr, offset, 4,
    //                                                 mask_raw & (0x0000FFFF << offset),
    //                                                 genPutPartialData(putdata));
    //               break;
    //             case 2:
    //               ulAgent->do_putpartialdata(addr, 0, 5, mask_raw & 0xFFFFFFFF,
    //                                                 genPutPartialData(putdata));
    //               break;
    //             }
    //             break;
    //     }
    // }
}


// void ULFuzzer::tick(std::shared_ptr<tl_agent::BaseAgent> *agent, int id, int mode) {
//   if(mode == true)
//     this->randomTest(agent);
//   else
//     this->caseTest(id);
// }

void ULFuzzer::tick(std::shared_ptr<tl_agent::BaseAgent> *agent, int id, int mode, std::shared_ptr<fake_l1::FakeL1> *l1) {
  if(mode == true)
    this->randomTest(agent, l1);
  else
    this->caseTest(id);
}
