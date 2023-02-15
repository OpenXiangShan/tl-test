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
  uint8_t *valid;
  uint8_t *ready;

  bool fire() const { return *valid && *ready; }
  virtual void free() = 0;
};

template <class Usr, class Echo, std::size_t N> class ChnA : public Decoupled {
public:
  uint8_t *opcode;
  uint8_t *param;
  uint8_t *size;
  uint32_t *source;
  uint32_t *mask;
  paddr_t *address;
  uint8_t *data;
  Usr *usr;
  Echo *echo;
  uint8_t *corrupt;
  uint8_t *alias;

  ChnA(){
    opcode = nullptr;
    param = nullptr;
    size = nullptr;
    source = nullptr;
    mask = nullptr;
    address = nullptr;
    data = nullptr;
    usr = nullptr;
    echo = nullptr;
    corrupt = nullptr;
    alias = nullptr;
  }

  void free() {
    delete (this->opcode);
    delete (this->param);
    delete (this->size);
    delete (this->source);
    delete (this->mask);
    delete (this->address);
    delete (this->alias);
  }
};

class ChnB : public Decoupled {
public:
  uint8_t *opcode;
  uint8_t *param;
  uint8_t *size;
  uint32_t *source;
  paddr_t *address;
  uint8_t *corrupt;
  uint8_t *alias;
  uint8_t *needdata;

  ChnB(){
    opcode = nullptr;
    param = nullptr;
    size = nullptr;
    source = nullptr;
    address = nullptr;
    corrupt = nullptr;
    alias = nullptr;
    needdata = nullptr;
  }

  void free() {
    delete (this->opcode);
    delete (this->param);
    delete (this->size);
    delete (this->source);
    delete (this->address);
    delete (this->alias);
  }
};

template <class Usr, class Echo, std::size_t N> class ChnC : public Decoupled {
public:
  uint8_t *opcode;
  uint8_t *param;
  uint8_t *size;
  uint32_t *source;
  paddr_t *address;
  Usr *usr;
  Echo *echo;
  uint8_t *dirty;
  uint8_t *data;
  uint8_t *corrupt;
  uint8_t *alias;

  ChnC(){
    opcode = nullptr;
    param = nullptr;
    size = nullptr;
    source = nullptr;
    address = nullptr;
    usr = nullptr;
    echo = nullptr;
    dirty = nullptr;
    data = nullptr;
    corrupt = nullptr;
    alias = nullptr;
  }

  void free() {
    delete (this->opcode);
    delete (this->param);
    delete (this->size);
    delete (this->source);
    delete (this->address);
    delete (this->alias);
  }
};

template <class Usr, class Echo, std::size_t N> class ChnD : public Decoupled {
public:
  uint8_t *opcode;
  uint8_t *param;
  uint8_t *size;
  uint32_t *source;
  uint8_t *sink;
  uint8_t *denied;
  Usr *usr;
  Echo *echo;
  uint8_t *dirty;
  uint8_t *data;
  uint8_t *corrupt;

  ChnD(){
    opcode = nullptr;
    param = nullptr;
    size = nullptr;
    source = nullptr;
    sink = nullptr;
    denied = nullptr;
    usr = nullptr;
    echo = nullptr;
    dirty = nullptr;
    data = nullptr;
    corrupt = nullptr;    
  }

  void free() {
    delete (this->opcode);
    delete (this->param);
    delete (this->size);
    delete (this->source);
    delete (this->sink);
  }
};

class ChnE : public Decoupled {
public:
  uint8_t *sink;
  paddr_t *addr; // used for index scoreboard
  uint8_t *alias;

  ChnE(){
    sink = nullptr;
    addr = nullptr; // used for index scoreboard
    alias = nullptr;
  }

  void free() {
    delete (this->sink);
    delete (this->addr);
    delete (this->alias);
  }
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
