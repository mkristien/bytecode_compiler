
CC:=g++
CC_OPT := -Og
LLVM_OPT := $(shell llvm-config --cxxflags)
LLVM_LIB := $(shell llvm-config --ldflags --system-libs --libs core)

# declare here all cpp files for the compiler (without the src/ prefix)
SRCS := compiler_main.cpp compiler.cpp
OBJS := $(SRCS:.cpp=.o)
SRCS := $(patsubst %, src/%, $(SRCS))
OBJS := $(patsubst %, bin/%, $(OBJS))

PROG_SRCS := wrapper.cpp runtime.cpp
PROG_OBJS := $(PROG_SRCS:.cpp=.o)
PROG_SRCS := $(patsubst %, src/%, $(SRCS))
PROG_OBJS := $(patsubst %, bin/%, $(PROG_OBJS))

DEPFLAGS = -MT $@ -MMD -MP -MF bin/$*.d

# use a provided wrapper to drive the input and output data
# link runtime with the generated machine code
program: $(PROG_OBJS) generated.o
	$(CC) -o $@ $^

$(PROG_OBJS): bin/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(DEPFLAGS) $(CC_OPT) -o $@ -c $<

generated.o : compiler
	./compiler

# build the compiler together with the example driver providing bytecode to compile
compiler: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LLVM_LIB)

$(OBJS): bin/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(DEPFLAGS) $(CC_OPT) $(LLVM_OPT) -o $@ -c $<

clean:
	rm -rf bin
	rm -f *.o
	rm -f compiler
	rm -f program

-include $(OBJS:.o=.d)
