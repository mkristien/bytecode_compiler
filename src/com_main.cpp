

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
      OP::LOAD, 1,
      OP::ADD,
      OP::STORE, 0,
      OP::STOP
    };
  }
  // bytecode.push_back(compiler::Opcode::STOP);

  compiler::Compiler c;
  c.ParseBytecode(bytecode);
  c.PrintIR();
  c.CompileIR();

  return 0;
}
