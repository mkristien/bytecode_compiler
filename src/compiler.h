
#pragma once

#include <memory>
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

  bool EmitObjectFile() const;

 private:
  // return EntryPoint function
  llvm::Function* InitialiseFunctions();

 private:
  std::unique_ptr<llvm::LLVMContext> ctx_;
  std::unique_ptr<llvm::Module>      mod_;
  std::unique_ptr<llvm::IRBuilder<>> builder_;

  bool compiled_success_;
};

}   // namespace compiler

