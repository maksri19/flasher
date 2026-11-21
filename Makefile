INCDIR := -I./include/
LIBDIR := 
RUNTIMEDIR := 

CFLAG := $(INCDIR) -Wall -Wextra -std=c17 -MMD -MP
LDFLAG := $(LIBDIR) $(RUNTIMEDIR) -lglfw -lGL -lm

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:src/%.c=build/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all clean

all: app


app: $(OBJS)
	clang $(OBJS) -o app $(LDFLAG)


build/%.o: src/%.c
	mkdir -p $(dir $@)
	clang $(CFLAG) $(INFLAG) -c $< -o $@


clean:
	rm -rf build/
	rm app


-include $(DEPS)
