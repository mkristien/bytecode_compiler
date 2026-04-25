
#pragma once

#include <cstdint>
#include <vector>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

namespace compiler {
class Compiler {
 public:
  Compiler();

  bool ParseBytecode(const std::vector<uint8_t>& bytecode);

  void PrintIR() const;

  void CompileIR() const;

 private:
  llvm::LLVMContext* ctx_;
  llvm::Module*      mod_;
  llvm::IRBuilder<>* builder_;
};

}   // namespace compiler

