CC=gcc
DEBUG_FLAGS=-g
OPTIMIZE_FLAGS=-O6
WARNING_FLAGS=-Wall
CFLAGS:=$(DEBUG_FLAGS) $(OPTIMIZE_FLAGS) $(WARNING_FLAGS)

OUTDIR = ../bin/

TESTS = \
    test_pdm \
    test_systems \

all:	$(TESTS)

test_pdm: test_pdm.c
	$(CC) $(CFLAGS) -o test_pdm test_pdm.c 
	mv test_pdm $(OUTDIR)

test_systems: test_systems.c
	$(CC) $(CFLAGS) -o test_systems test_systems.c 
	mv test_systems $(OUTDIR)

clean:
	rm -f *.o *.dat *~ $(TESTS)