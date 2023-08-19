prog=test

prog?=$(basename $(notdir $(PWD)))
objs+=$(addsuffix .o,$(basename $(wildcard *.c *.cc)))

defines+=-D_GNU_SOURCE
ccflags+=-g -Wall -Wno-int-conversion -MMD $(defines)
ldflags+=-g

.SUFFIXES:

%.o: %.c  ; gcc -o $@ -c $< $(ccflags) -std=c99
%.i: %.c  ; gcc -o $@ -E $< $(defines)
%.o: %.cc ; g++ -o $@ -c $< $(ccflags)
%.i: %.cc ; g++ -o $@ -E $< $(defines)

ld?=gcc

$(prog): $(objs) ; $(ld) -o $@ $^ $(ldflags)

.PHONY: clean run valgrind

clean: ; rm -f $(prog) *.o *.d *.i

run:      $(prog) ; ./$< $(args)
valgrind: $(prog) ; $@ --leak-check=full --show-leak-kinds=all ./$< $(args)

sinclude *.d
