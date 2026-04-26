# bytecode_compiler

This project implements a simple compiler of bytecode for a virtual stack machine.
The compiler processes a sequence of bytecodes and emits machine code that implements
the program described by the input bytecode as a function with the following signature:

`void f(uint8_t* in, uint8_t* out)`

## Quick Start
```
$ make compiler   # create the compiler and its driver with an example bytecode program
$ ./compiler      # run the compiler to parse the example bytecode
$ make program    # compiler the program driver / wrapper with the freshly generated machine code
$ ./program <data.in> <data.out>    # run the created program with the given input file
```
## Quicker Start
```
$ make
$ ./program <data.in> <data.out>    # run the created program with the given input file
```

## Project Organisation

This project comprises two distinct set of source-code files.
1. compiler - processes bytecode, generetas LLVM IR, emits machine code
2. program - produces final executable by linking a data wrapper with the compiler-generated code

### Compiler

The primary implementation of the bytecode compiler is provided in `compiler.[h|cpp]`.

`bool Compiler::ParseBytecode(const std::vector<uint8_t>& bytecode)`

Given a vector of bytecode (opcodes and data), the compiler constructs LLVM IR
corresponding to the given program. This function return true iff the bytecode was
valid and IR was constructed successfully.

`bool Compiler::EmitObjectFile()`

After the LLVM IR is constructed, the program can be compiled into machine code,
stored in an object file `generated.o`.


`compiler_main.cpp` is provided as an example driver of the compiler. It instantiates
the compiler objects, feeds it a sequence of bytecode, prints the generated LLVM IR,
and initiates compilation into machine code stored in an object file.

### Program

The main program is constructed by combining the `generated.o` machine code
corresponding to the input bytecode and the stack-machine `runtime.cpp` that implements
the virtual machine.

Furthermore, `wrapper.cpp` is provided to read input data, drive the program function,
and store the result in an output data file.
