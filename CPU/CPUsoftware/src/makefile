CC=g++
OPTIMIZE_FLAGS=-O3
WARNING_FLAGS=-Wall
INCLUDE_FLAGS:=-I../include
LIB_FLAGS:=-L../lib -lrtd-dm75xx -std=c++11 -pthread
CFLAGS:=$(INCLUDE_FLAGS) $(OPTIMIZE_FLAGS) $(WARNING_FLAGS) $(LIB_FLAGS)

OUTDIR = ../bin/

TESTS = \
	test_system \
	no_hv \

all:    $(TESTS)

test_system: test_system.cpp
	$(CC) $(CFLAGS) -o test test_system.cpp globals.cpp cpu_functions.cpp general.cpp 
	mv test $(OUTDIR)

no_hv: no_hv.cpp
	$(CC) $(CFLAGS) -o no_hv no_hv.cpp globals.cpp cpu_functions.cpp general.cpp 
	mv no_hv $(OUTDIR)

clean:
	rm -f *.o *.dat *~ $(TESTS)
