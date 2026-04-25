
#include "compiler.h"

#include <cstdio>

#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Host.h>

#include "opcodes.h"

using namespace std;
using namespace llvm;

namespace compiler {

Compiler::Compiler()
: ctx_(make_unique<LLVMContext>())
, compiled_success_(false) {
}

Function* Compiler::InitialiseFunctions() {
  //////////////////////////////////////////////////////////////////////////////
  // declare runtime functions (to be linked separatelly)
  //////////////////////////////////////////////////////////////////////////////
  // LOAD, STORE
  FunctionType* ptr_type = FunctionType::get(Type::getVoidTy(*ctx_), PointerType::get(Type::getInt8Ty(*ctx_), 0), false);
  Function::Create(ptr_type, Function::ExternalLinkage, "load",  mod_.get());
  Function::Create(ptr_type, Function::ExternalLinkage, "store", mod_.get());
  // POP, ADD, SUB, DUP
  FunctionType* void_type = FunctionType::get(Type::getVoidTy(*ctx_), 0);
  Function::Create(void_type, Function::ExternalLinkage, "pop", mod_.get());
  Function::Create(void_type, Function::ExternalLinkage, "add", mod_.get());
  Function::Create(void_type, Function::ExternalLinkage, "sub", mod_.get());
  Function::Create(void_type, Function::ExternalLinkage, "dup", mod_.get());


  //////////////////////////////////////////////////////////////////////////////
  // declare the entry point function "void f(unsigned char* in, unsigned char* out);"
  auto ptr_t  = PointerType::get(Type::getInt8Ty(*ctx_), 0);
  auto void_t = Type::getVoidTy(*ctx_);
  FunctionType* entry_signature = FunctionType::get(void_t, {ptr_t, ptr_t}, false);
  Function*     entry_function  = Function::Create(entry_signature, Function::ExternalLinkage, "f", mod_.get());
  entry_function->getArg(0)->setName("in");
  entry_function->getArg(1)->setName("out");
  return entry_function;
}

bool Compiler::ParseBytecode(const std::vector<uint8_t>& bytecode) {
  compiled_success_ = false;
  mod_              = make_unique<Module>("bytecode", *ctx_);
  builder_          = make_unique<IRBuilder<>>(*ctx_);

  // start generating IR
  Function* entry_point = InitialiseFunctions();
  builder_->SetInsertPoint(BasicBlock::Create(*ctx_, "", entry_point));

  // Generate IR to compute a buffer offset given an index
  auto GetOffset = [&] (uint8_t index) -> Value* {
    Value* index_val  = ConstantInt::get(*ctx_, APInt(16, index));
    Value* word_size  = ConstantInt::get(*ctx_, APInt(16, kWordSize));
    Value* offset     = builder_->CreateMul(index_val, word_size);
    return offset;
  };

  int i          = 0;                   // index into bytecode to consume
  int s          = bytecode.size();     // total size of bytecode
  int stack_size = 0;                   // tracking runtime stack size to generate compiler errors
  while (i < s) {
    switch (bytecode[i]) {
      case STOP: {
        builder_->CreateRetVoid();
        compiled_success_ = true;
        return compiled_success_;
      }
      case LOAD: {
        // compute the address of in[index]
        Value* address = builder_->CreateAdd(entry_point->getArg(0), GetOffset(bytecode[++i]));

        // call runtime function
        Function* callee = mod_->getFunction("load");
        builder_->CreateCall(callee, {address});
        stack_size++;
        break;
      }
      case STORE: {
        if (stack_size < 1) {
          printf("error: trying to store from an empty stack\n");
          return compiled_success_;
        }
        // compute the address of in[index]
        Value* address = builder_->CreateAdd(entry_point->getArg(1), GetOffset(bytecode[++i]));

        // call runtime function
        Function* callee = mod_->getFunction("store");
        builder_->CreateCall(callee, {address});
        break;
      }
      case POP: {
        if (stack_size < 1) {
          printf("error: trying to pop from an empty stack\n");
          return compiled_success_;
        }
        // call runtime function
        Function* callee = mod_->getFunction("pop");
        builder_->CreateCall(callee);
        stack_size--;
        break;
      }
      case ADD: {
        if (stack_size < 2) {
          printf("error: trying to add must have stack size of at least 2\n");
          return compiled_success_;
        }
        // call runtime function
        Function* callee = mod_->getFunction("add");
        builder_->CreateCall(callee);
        stack_size--;
        break;
      }
      case SUB: {
        if (stack_size < 2) {
          printf("error: trying to sub must have stack size of at least 2\n");
          return compiled_success_;
        }
        // call runtime function
        Function* callee = mod_->getFunction("sub");
        builder_->CreateCall(callee);
        stack_size--;
        break;
      }
      case DUP: {
        if (stack_size < 1) {
          printf("error: trying to dub on an empty stack\n");
          return compiled_success_;
        }
        // call runtime function
        Function* callee = mod_->getFunction("dup");
        builder_->CreateCall(callee);
        stack_size++;
        break;
      }
      default: {
        printf("warn: unknown bytecode 0x%x\n", bytecode[i]);
      }
    }
    i++;
  }
  printf("error: end of bytecode without STOP\n");
  return compiled_success_;
}

void Compiler::PrintIR() const {
  if (mod_ == 0) {
    printf("no code has been compiled\n");
    return;
  }

  if (!compiled_success_) {
    printf("no bytecode has been successfully compiled\n");
    return;
  }

  mod_->print(errs(), 0);
}

bool Compiler::EmitObjectFile() const {
  if (!compiled_success_) {
    printf("no bytecode has been successfully compiled\n");
    return false;
  }

  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmParsers();
  InitializeAllAsmPrinters();
  std::string Error;
  auto TargetTriple = sys::getDefaultTargetTriple();
  auto Target       = TargetRegistry::lookupTarget(TargetTriple, Error);

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
  std::error_code EC;
  raw_fd_ostream dest("generated.o", EC, sys::fs::OF_None);

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
