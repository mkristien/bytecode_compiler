
#include "compiler.h"

#include <cstdio>


#include "llvm/ADT/APInt.h"
// #include "llvm/ADT/STLExtras.h"
// #include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
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
  // declare helper functions (to be linked separatelly)
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
  vector<Type*> entry_arguments (2, PointerType::get(Type::getInt8Ty(*ctx_), 0));
  FunctionType* entry_signature = FunctionType::get(Type::getVoidTy(*ctx_), entry_arguments, false);
  Function*     entry_function  = Function::Create(entry_signature, Function::ExternalLinkage, "f", mod_.get());
  auto args_it = entry_function->args().begin();
  args_it->setName("in");
  args_it++;
  args_it->setName("out");
  return entry_function;
}

bool Compiler::ParseBytecode(const std::vector<uint8_t>& bytecode) {
  mod_     = make_unique<Module>("bytecode", *ctx_);

  // start generating IR
  Function* entry_point = InitialiseFunctions();
  builder_ = make_unique<IRBuilder<>>(*ctx_);
  BasicBlock* BB = BasicBlock::Create(*ctx_, "entry", entry_point);
  builder_->SetInsertPoint(BB);

  auto GetOffset = [&] (uint8_t index) -> Value* {
    Value* index_val  = ConstantInt::get(*ctx_, APInt(16, index));
    Value* word_size  = ConstantInt::get(*ctx_, APInt(16, kWordSize));
    Value* offset     = builder_->CreateMul(index_val, word_size);
    return offset;
  };

  int i = 0;
  int s = bytecode.size();
  int stack_size = 0;
  compiled_success_ = false;
  while (i < s) {
    switch (bytecode[i]) {
      case STOP: {
        builder_->CreateRetVoid();
        compiled_success_ = true;
        return compiled_success_;
      }
      case LOAD: {
        Function* callee = mod_->getFunction("load");
        if (!callee) {
          printf("warn: function load has not been declared\n");
          return compiled_success_;
        }

        // call load helper function
        Value* address = builder_->CreateAdd(entry_point->getArg(0), GetOffset(bytecode[++i]));
        builder_->CreateCall(callee, {address});
        stack_size++;
        break;
      }
      case STORE: {
        if (stack_size < 1) {
          printf("error: trying to store from an empty stack\n");
          return compiled_success_;
        }
        Function* callee = mod_->getFunction("store");
        if (!callee) {
          printf("warn: function store has not been declared\n");
          return compiled_success_;
        }

        // call helper function
        Value* address = builder_->CreateAdd(entry_point->getArg(1), GetOffset(bytecode[++i]));
        builder_->CreateCall(callee, {address});
        break;
      }
      case POP: {
        if (stack_size < 1) {
          printf("error: trying to pop from an empty stack\n");
          return compiled_success_;
        }
        Function* callee = mod_->getFunction("pop");
        if (!callee) {
          printf("warn: function pop has not been declared\n");
          return compiled_success_;
        }

        // call helper function
        builder_->CreateCall(callee);
        stack_size--;
        break;
      }
      case ADD: {
        if (stack_size < 2) {
          printf("error: trying to add must have stack size of at least 2\n");
          return compiled_success_;
        }
        Function* callee = mod_->getFunction("add");
        if (!callee) {
          printf("warn: function add has not been declared\n");
          return compiled_success_;
        }

        // call helper function
        builder_->CreateCall(callee);
        stack_size--;
        break;
      }
      case SUB: {
        if (stack_size < 2) {
          printf("error: trying to sub must have stack size of at least 2\n");
          return compiled_success_;
        }
        Function* callee = mod_->getFunction("sub");
        if (!callee) {
          printf("warn: function sub has not been declared\n");
          return compiled_success_;
        }

        // call helper function
        builder_->CreateCall(callee);
        stack_size--;
        break;
      }
      case DUP: {
        if (stack_size < 1) {
          printf("error: trying to dub on an empty stack\n");
          return compiled_success_;
        }
        Function* callee = mod_->getFunction("dup");
        if (!callee) {
          printf("warn: function dup has not been declared\n");
          return compiled_success_;
        }

        // call helper function
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
  builder_->CreateRetVoid();
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

bool Compiler::CompileIR() const {
  if (!compiled_success_) {
    printf("no bytecode has been successfully compiled\n");
    return;
  }

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
