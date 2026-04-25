
CC:=g++
CC_OPT := -Og
LLVM_OPT := $(shell llvm-config --cxxflags)
LLVM_LIB := $(shell llvm-config --ldflags --system-libs --libs core)

# declare here all cpp files for the compiler (without the src/ prefix)
SRCS := compiler_main.cpp compiler.cpp
OBJS := $(SRCS:.cpp=.o)

SRCS := $(patsubst %, src/%, $(SRCS))
OBJS := $(patsubst %, bin/%, $(OBJS))
DEPFLAGS = -MT $@ -MMD -MP -MF bin/$*.d

# use a provided wrapper to drive the input and output data
# link runtime with the generated machine code
program: wrapper.cpp runtime.cpp generated.o
	$(CC) -o $@ $^ $(CC_OPT)

# build the compiler together with the example driver providing bytecode to compile
compiler: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LLVM_LIB)

$(OBJS): bin/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(DEPFLAGS) $(CC_OPT) $(LLVM_OPT) -o $@ -c $<

clean:
	rm -rf bin
	rm -f *.o

-include $(OBJS:.o=.d)
