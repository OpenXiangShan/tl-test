//
// Created by ljw on 10/21/21.
//

#ifndef TLC_TEST_PORT_H
#define TLC_TEST_PORT_H

#include "../Utils/Common.h"
#include <array>
#include <cstddef>
#include <cstdint>

namespace tl_agent {

enum {
  PutFullData = 0,
  PutPartialData,
  ArithmeticData,
  LogicalData,
  Get,
  Hint,
  AcquireBlock,
  AcquirePerm
};
enum { Probe = 6 };
enum {
  AccessAck = 0,
  AccessAckData,
  HintAck,
  ProbeAck = 4,
  ProbeAckData,
  Release,
  ReleaseData
};
enum { Grant = 4, GrantData, ReleaseAck };
enum { GrantAck = 0 };

enum { toT = 0, toB, toN };
enum { NtoB = 0, NtoT, BtoT };
enum { TtoB = 0, TtoN, BtoN, TtoT, BtoB, NtoN };
enum { INVALID = 0, BRANCH, TRUNK, TIP };

class Decoupled {
public:
  std::shared_ptr<uint8_t> valid;
  std::shared_ptr<uint8_t> ready;

  bool fire() const { return *valid && *ready; }
};

template <class Usr, class Echo, std::size_t N> class ChnA : public Decoupled {
public:
  std::shared_ptr<uint8_t> opcode;
  std::shared_ptr<uint8_t> param;
  std::shared_ptr<uint8_t> size;
  std::shared_ptr<uint32_t> source;
  std::shared_ptr<uint32_t> mask;
  std::shared_ptr<paddr_t> address;
  std::shared_ptr<uint8_t[]> data;
  std::shared_ptr<Usr> usr;
  std::shared_ptr<Echo> echo;
  std::shared_ptr<uint8_t> corrupt;
  std::shared_ptr<uint8_t> alias;
};

class ChnB : public Decoupled {
public:
  std::shared_ptr<uint8_t> opcode;
  std::shared_ptr<uint8_t> param;
  std::shared_ptr<uint8_t> size;
  std::shared_ptr<uint32_t> source;
  std::shared_ptr<paddr_t> address;
  std::shared_ptr<uint8_t> corrupt;
  std::shared_ptr<uint8_t> alias;
  std::shared_ptr<uint8_t> needdata;
};

template <class Usr, class Echo, std::size_t N> class ChnC : public Decoupled {
public:
  std::shared_ptr<uint8_t> opcode;
  std::shared_ptr<uint8_t> param;
  std::shared_ptr<uint8_t> size;
  std::shared_ptr<uint32_t> source;
  std::shared_ptr<paddr_t> address;
  std::shared_ptr<Usr> usr;
  std::shared_ptr<Echo> echo;
  std::shared_ptr<uint8_t> dirty;
  std::shared_ptr<uint8_t[]> data;
  std::shared_ptr<uint8_t> corrupt;
  std::shared_ptr<uint8_t> alias;
};

template <class Usr, class Echo, std::size_t N> class ChnD : public Decoupled {
public:
  std::shared_ptr<uint8_t> opcode;
  std::shared_ptr<uint8_t> param;
  std::shared_ptr<uint8_t> size;
  std::shared_ptr<uint32_t> source;
  std::shared_ptr<uint32_t> sink;
  std::shared_ptr<uint8_t> denied;
  std::shared_ptr<Usr> usr;
  std::shared_ptr<Echo> echo;
  std::shared_ptr<uint8_t> dirty;
  std::shared_ptr<uint8_t[]> data;
  std::shared_ptr<uint8_t> corrupt;
};

class ChnE : public Decoupled {
public:
  std::shared_ptr<uint32_t> sink;
  std::shared_ptr<paddr_t> addr; // used for index scoreboard
  std::shared_ptr<uint8_t> alias;
};

template <class ReqField, class RespField, class EchoField, std::size_t N>
class Port {
public:
  ChnA<ReqField, EchoField, N> a;
  ChnB b;
  ChnC<ReqField, EchoField, N> c;
  ChnD<RespField, EchoField, N> d;
  ChnE e;
};

} // namespace tl_agent

#endif // TLC_TEST_PORT_H
