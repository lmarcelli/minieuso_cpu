CC=gcc
DEBUG_FLAGS=-g
INCLUDE_FLAGS=-I../include
LIBRARY_FLAGS=-L../lib -lrtd-dm75xx  -lm -lpthread
OPTIMIZE_FLAGS=-O6
WARNING_FLAGS=-Wall
CFLAGS:=$(DEBUG_FLAGS) $(INCLUDE_FLAGS) $(OPTIMIZE_FLAGS) $(WARNING_FLAGS)

TESTS = \
    test_photodiode \
    test_pdm_photodiode \
    test_dio \
    test_lvps_switch \

all:	$(TESTS)

test_photodiode: test_photodiode.c
	$(CC) $(CFLAGS) -o ../bin/test_photodiode test_photodiode.c $(LIBRARY_FLAGS)

test_pdm_photodiode: test_pdm_photodiode.c
	$(CC) $(CFLAGS) -o ../bin/test_pdm_photodiode test_pdm_photodiode.c $(LIBRARY_FLAGS)

test_dio: test_dio.c
	$(CC) $(CFLAGS) -o ../bin/test_dio test_dio.c $(LIBRARY_FLAGS)

test_dio: test_lvps_switch.c
	$(CC) $(CFLAGS) -o ../bin/test_lvps_switch test_lvps_switch.c $(LIBRARY_FLAGS)

clean:
	rm -f *.o *.dat *~ $(TESTS)
