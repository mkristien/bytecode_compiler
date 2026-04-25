
#include "compiler.h"

#include <cstdio>


// #include "llvm/ADT/APFloat.h"
// #include "llvm/ADT/STLExtras.h"
// #include "llvm/IR/BasicBlock.h"
// #include "llvm/IR/Constants.h"
// #include "llvm/IR/DerivedTypes.h"
// #include "llvm/IR/Function.h"
// #include "llvm/IR/Type.h"
// #include "llvm/IR/Verifier.h"

namespace compiler {

Compiler::Compiler() {
   ctx_     = new llvm::LLVMContext();
  }
  
  
  
  
  bool Compiler::ParseBytecode(const std::vector<uint8_t>& bytecode) {
  mod_     = new llvm::Module("bytecode file", *ctx_);
  builder_ = new llvm::IRBuilder(*ctx_);
  return true;
}

void Compiler::PrintIR() const {
  if (mod_ == 0) {
    printf("no code has been compiled\n");
    return;
  }
  
  mod_->print(llvm::errs(), 0);
}

void Compiler::CompileIR() const {

}

}   // namespace compiler
