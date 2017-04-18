CC=g++
OPTIMIZE_FLAGS=-O3
WARNING_FLAGS=-Wall
LIB_FLAGS=-stdc++11
CFLAGS:=$(DEBUG_FLAGS) $(OPTIMIZE_FLAGS) $(WARNING_FLAGS)

OUTDIR = ../bin/

TESTS = \
	test_system \

all:    $(TESTS)

test_system: test_system.cpp
	$(CC) $(CFLAGS) -o test test_system.cpp globals.cpp cpu_functions.cpp general.cpp 
	mv test $(OUTDIR)

clean:
	rm -f *.o *.dat *~ $(TESTS)
