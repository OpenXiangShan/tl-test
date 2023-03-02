//
// Created by wkf on 2021/10/29.
//

#ifndef TLC_TEST_FUZZER_H
#define TLC_TEST_FUZZER_H

#include "../TLAgent/CAgent.h"
#include "../TLAgent/ULAgent.h"
#include "../Fuzzer/Case.h"
#include "../CacheModel/FakeL1/fake_l1.h"
#include "../CacheModel/FakePTW/fake_ptw.h"

class Fuzzer {
protected:
  uint64_t *cycles;
  testcase::Message case_mes[testcase::N_TESTCASE_AGENT];

public:
  Fuzzer() = default;
  ~Fuzzer() = default;
  // virtual void tick(std::shared_ptr<tl_agent::BaseAgent> *agent, int id , int mode) = 0;
  virtual void tick(std::shared_ptr<tl_agent::BaseAgent> *agent, int id , int mode, std::shared_ptr<fake_l1::FakeL1> *l1) = 0;
  void set_cycles(uint64_t *cycles) { this->cycles = cycles; }

  void init_testcase(void){
    testcase::read_file rt;
    rt.read_test(case_mes);
  }

  bool do_reset(uint64_t Cycles){
    if(case_mes[0].haskey(Cycles)){
      if(case_mes[0].query(Cycles).opcode == testcase::reset_opcode){
        std::cout << "System: "<< "[" << Cycles << "] " << "reset" << std::endl;
        return true;
      }
    }
    return false;
  }

  paddr_t genAddr() {
    return (rand() % 0x8) * (rand() % 0x80) * 0x40 + DRAM_OFFSET; // Tag + Set + Offset
  }
};

class ULFuzzer : public Fuzzer {
private:
  std::shared_ptr<tl_agent::ULAgent> ulAgent;

public:
  ULFuzzer(std::shared_ptr<tl_agent::ULAgent> ulAgent);
  // void randomTest(std::shared_ptr<tl_agent::BaseAgent> *agent);
  void randomTest(std::shared_ptr<tl_agent::BaseAgent> *agent, std::shared_ptr<fake_l1::FakeL1> *l1);

  void caseTest(int id);
  // void tick(std::shared_ptr<tl_agent::BaseAgent> *agent, int id, int mode);
  void tick(std::shared_ptr<tl_agent::BaseAgent> *agent, int id, int mode, std::shared_ptr<fake_l1::FakeL1> *l1);
};

class CFuzzer : public Fuzzer {
private:
  std::shared_ptr<tl_agent::CAgent> cAgent;

public:
  CFuzzer(std::shared_ptr<tl_agent::CAgent> cAgent);
  void randomTest(bool do_alias, std::shared_ptr<tl_agent::BaseAgent> *agent);
  void caseTest(int id);
  void tick(std::shared_ptr<tl_agent::BaseAgent> *agent, int id, int mode);
};

// tl_base_agent::TLCTransaction randomTest2(bool do_alias, uint8_t bus_type, std::shared_ptr<tl_agent::BaseAgent> *agent);


tl_base_agent::TLCTransaction randomTest2(bool do_alias, uint8_t bus_type, std::shared_ptr<fake_ptw::FakePTW> *ptw, std::shared_ptr<fake_ptw::FakePTW> *dma);

tl_base_agent::TLCTransaction randomTest3(std::shared_ptr<fake_ptw::FakePTW> *ptw, std::shared_ptr<fake_ptw::FakePTW> *dma,std::shared_ptr<fake_l1::FakeL1> *l1, int bus_type);

#endif // TLC_TEST_FUZZER_H
