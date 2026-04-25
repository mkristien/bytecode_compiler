
#include "compiler.h"

#include <cstdio>


// #include "llvm/ADT/APFloat.h"
// #include "llvm/ADT/STLExtras.h"
// #include "llvm/IR/BasicBlock.h"
// #include "llvm/IR/Constants.h"
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
// #include "llvm/IR/Function.h"
// #include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"

#include "llvm/Support/FileSystem.h"
// #include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "llvm/MC/TargetRegistry.h"
#include "llvm/TargetParser/Host.h"

using namespace std;
using namespace llvm;

namespace compiler {

Compiler::Compiler() {
  ctx_ = make_unique<LLVMContext>();
}

bool Compiler::ParseBytecode(const std::vector<uint8_t>& bytecode) {
  mod_     = make_unique<Module>("bytecode file", *ctx_);
  builder_ = make_unique<IRBuilder<>>(*ctx_);

  vector<Type*> entry_arguments (2, PointerType::get(Type::getInt8Ty(*ctx_), 0));
  FunctionType* entry_signature = FunctionType::get(Type::getVoidTy(*ctx_), entry_arguments, false);
  Function*     entry_function  = Function::Create(entry_signature, Function::ExternalLinkage, "f", mod_.get());
  auto args_it = entry_function->args().begin();
  args_it->setName("in");
  args_it++;
  args_it->setName("out");

  BasicBlock* BB = BasicBlock::Create(*ctx_, "entry", entry_function);
  builder_->SetInsertPoint(BB);

  builder_->CreateRetVoid();
  return true;
}

void Compiler::PrintIR() const {
  if (mod_ == 0) {
    printf("no code has been compiled\n");
    return;
  }

  mod_->print(errs(), 0);
}

bool Compiler::CompileIR() const {
  auto TargetTriple = sys::getDefaultTargetTriple();
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmParsers();
  InitializeAllAsmPrinters();
  std::string Error;
  auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

  // Print an error and exit if we couldn't find the requested target.
  if (!Target) {
    errs() << Error;
    return false;
  }

  TargetOptions opt;
  auto TargetMachine = Target->createTargetMachine(TargetTriple, "generic", "", opt, Reloc::PIC_);

  // configure Module with the new target
  mod_->setDataLayout(TargetMachine->createDataLayout());
  mod_->setTargetTriple(TargetTriple);

  // emit object file
  auto Filename = "generated.o";
  std::error_code EC;
  raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);

  if (EC) {
    errs() << "Could not open file: " << EC.message();
    return false;
  }

  legacy::PassManager pass;
  if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, CodeGenFileType::ObjectFile)) {
    errs() << "TargetMachine can't emit a file of this type";
    return false;
  }

  pass.run(*mod_);
  dest.flush();
  return true;
}

}   // namespace compiler
