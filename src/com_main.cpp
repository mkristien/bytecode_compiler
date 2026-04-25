

#include <cstdio>
#include <vector>

#include "opcodes.h"
#include "compiler.h"

int main() {
  std::vector<uint8_t> bytecode;
  bytecode.push_back(compiler::Opcode::STOP);

  compiler::Compiler c;
  c.ParseBytecode(bytecode);
  c.PrintIR();

  return 0;
}
