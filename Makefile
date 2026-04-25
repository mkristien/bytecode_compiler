
CC=g++
CC_OPT= -Og -fpermissive

# declare here all cpp files for the compiler (without the src/ prefix)
SRCS := com_main.cpp
OBJS := $(SRCS:.cpp=.o)

SRCS := $(patsubst %, src/%, $(SRCS))
OBJS := $(patsubst %, bin/%, $(OBJS))
DEPFLAGS = -MT $@ -MMD -MP -MF bin/$*.d

program: wrapper.cpp generated.o
	$(CC) -o $@ $^ $(CC_OPT)

compiler: $(OBJS)
	$(CC) -o $@ $(OBJS)

$(OBJS): bin/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(DEPFLAGS) $(CC_OPT) -o $@ -c $<

clean:
	rm -r bin

-include $(OBJS:.o=.d)
