//
// Created by wkf on 2021/10/29.
//

#ifndef TLC_TEST_FUZZER_H
#define TLC_TEST_FUZZER_H

#include "../TLAgent/CAgent.h"
#include "../TLAgent/ULAgent.h"

class Fuzzer {
protected:
  uint64_t *cycles;

public:
  Fuzzer() = default;
  ~Fuzzer() = default;
  virtual void tick(std::shared_ptr<tl_agent::BaseAgent> *agent) = 0;
  void set_cycles(uint64_t *cycles) { this->cycles = cycles; }
};

class ULFuzzer : public Fuzzer {
private:
  tl_agent::ULAgent *ulAgent;

public:
  ULFuzzer(tl_agent::ULAgent *ulAgent);
  void randomTest(std::shared_ptr<tl_agent::BaseAgent> *agent);
  void caseTest();
  void caseTest2();
  void tick(std::shared_ptr<tl_agent::BaseAgent> *agent);
};

class CFuzzer : public Fuzzer {
private:
  std::shared_ptr<tl_agent::CAgent> cAgent;

public:
  CFuzzer(std::shared_ptr<tl_agent::CAgent> cAgent);
  void randomTest(bool do_alias, std::shared_ptr<tl_agent::BaseAgent> *agent);
  void caseTest();
  void tick(std::shared_ptr<tl_agent::BaseAgent> *agent);
};

#endif // TLC_TEST_FUZZER_H
