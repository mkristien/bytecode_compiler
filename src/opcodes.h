
#pragma once

#include <stdint.h>

namespace compiler {

enum Opcode : uint8_t {
  STOP  = 0x00,
  LOAD  = 0x01,
  STORE = 0x02,
  POP   = 0x03,
  ADD   = 0x04,
  SUB   = 0x05,
  DUP   = 0x06,
};

}   // namespace compiler
