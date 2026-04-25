

#include <cstdio>
#include <vector>

#include "opcodes.h"
#include "compiler.h"

int main() {
  std::vector<uint8_t> bytecode;
  {
    using OP = compiler::Opcode;
    bytecode = {
      OP::LOAD, 0,
      OP::DUP,
      OP::SUB,
      OP::STORE, 0,
      OP::POP,
      OP::STOP
    };
  }

  compiler::Compiler c;
  if (!c.ParseBytecode(bytecode)) {
    printf("error: could not compile bytecode\n");
    return 1;
  }

  c.PrintIR();
  if (!c.EmitObjectFile()) {
    printf("error: could not generate object file\n");
    return 1;
  }

  return 0;
}
