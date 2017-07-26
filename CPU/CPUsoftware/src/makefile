CC=g++
OPTIMIZE_FLAGS=-O3
DEBUG_FLAGS=-g
WARNING_FLAGS=-Wall
INCLUDE_FLAGS:=-I../include
LIB_FLAGS:=-L../lib -lrtd-dm75xx -lm -lpthread -lusb-1.0
CFLAGS:=-std=c++11 $(INCLUDE_FLAGS) $(OPTIMIZE_FLAGS) $(WARNING_FLAGS) $(DEBUG_FLAGS)

OUTDIR = ../bin/

TESTS = \
	no_hv \

all:    $(TESTS)

no_hv: no_hv.cpp
	$(CC) $(CFLAGS) no_hv.cpp globals.cpp cpu_functions.cpp general.cpp usb_interface.cpp $(LIB_FLAGS) -o no_hv 
	mv no_hv $(OUTDIR)

clean:
	rm -f *.o *.dat *~ $(TESTS)
