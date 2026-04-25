
CC:=g++
CC_OPT := -Og -fpermissive
LLVM_OPT := $(shell llvm-config --cxxflags)
LLVM_LIB := $(shell llvm-config --ldflags --system-libs --libs core)

# declare here all cpp files for the compiler (without the src/ prefix)
SRCS := com_main.cpp compiler.cpp
OBJS := $(SRCS:.cpp=.o)

SRCS := $(patsubst %, src/%, $(SRCS))
OBJS := $(patsubst %, bin/%, $(OBJS))
DEPFLAGS = -MT $@ -MMD -MP -MF bin/$*.d

program: wrapper.cpp runtime.cpp generated.o
	$(CC) -o $@ $^ $(CC_OPT)

compiler: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LLVM_LIB)

$(OBJS): bin/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(DEPFLAGS) $(CC_OPT) $(LLVM_OPT) -o $@ -c $<

clean:
	rm -r bin

-include $(OBJS:.o=.d)
