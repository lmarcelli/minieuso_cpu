CC=g++
OPTIMIZE_FLAGS=-O3
DEBUG_FLAGS=-g
WARNING_FLAGS=-Wall
INCLUDE_FLAGS:=-I../include
LIB_FLAGS:=-L../lib -lrtd-dm75xx -lm -lpthread
CFLAGS:=-std=c++11 $(INCLUDE_FLAGS) $(OPTIMIZE_FLAGS) $(WARNING_FLAGS) $(DEBUG_FLAGS)

OUTDIR = ../bin/

TESTS = \
	test_system \
	no_hv \

all:    $(TESTS)

test_system: test_system.cpp
	$(CC) $(CFLAGS) test_system.cpp globals.cpp cpu_functions.cpp general.cpp $(LIB_FLAGS) -o test 
	mv test $(OUTDIR)

no_hv: no_hv.cpp
	$(CC) $(CFLAGS) no_hv.cpp globals.cpp cpu_functions.cpp general.cpp $(LIB_FLAGS) -o no_hv 
	mv no_hv $(OUTDIR)

clean:
	rm -f *.o *.dat *~ $(TESTS)
