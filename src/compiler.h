
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

  /**
   * Provided a sequence of bytecode (opcodes and operands), this method
   * generates an LLVM IR for this Compiler object.
   */
  bool ParseBytecode(const std::vector<uint8_t>& bytecode);

  /**
   * Show the internal IR, if already generated.
   */
  void PrintIR() const;

  /**
   * Output the internal IR to an object file "generated.o".
   * This file provides the machine code for "void f(uint8*, uint8*)" that
   * can be linked into a program with a wrapper prividing in/out buffers.
   */
  bool EmitObjectFile() const;

 private:
  /**
   * Declare runtime functions and the main entry-point "void f(uint8*, uint8*)"
   *
   * @return entry-point function
   */
  llvm::Function* InitialiseFunctions();

 private:
  std::unique_ptr<llvm::LLVMContext> ctx_;
  std::unique_ptr<llvm::Module>      mod_;
  std::unique_ptr<llvm::IRBuilder<>> builder_;

  bool compiled_success_;
};

}   // namespace compiler

