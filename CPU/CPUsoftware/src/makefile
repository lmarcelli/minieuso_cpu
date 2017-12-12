platform=$(shell uname -s)

CC=gcc
CXX=g++
OPTIMIZE_FLAGS=-O3
DEBUG_FLAGS=-g
WARNING_FLAGS=-Wall
INCLUDE_FLAGS:=-I../include

ifeq ($(platform), Darwin)
LIB_FLAGS:=-L../lib -lm -lpthread -lusb-1.0
CFLAGS:=-std=c++11 $(INCLUDE_FLAGS) $(OPTIMIZE_FLAGS) $(WARNING_FLAGS) 
else
LIB_FLAGS:=-L../lib -lrtd-dm75xx -lrtd-aDIO -lm -lpthread -lusb-1.0
CFLAGS:=-std=c++11 $(INCLUDE_FLAGS) $(OPTIMIZE_FLAGS) $(WARNING_FLAGS) $(DEBUG_FLAGS)
endif
SOURCES := $(shell find . -name '*.cpp' -and -not -name "*lvps_switch*")

OUTDIR = ../bin/

TESTS = \
	mecontrol \
	lvps_switch \

all:    $(TESTS)

mecontrol: mecontrol.cpp
	$(CXX) $(CFLAGS) $(SOURCES) $(LIB_FLAGS) -o mecontrol 
	mv mecontrol $(OUTDIR)

lvps_switch: lvps_switch.cpp
	$(CXX) $(CFLAGS) lvps_switch.cpp LvpsManager.cpp $(LIB_FLAGS) -o lvps_switch 
	mv lvps_switch $(OUTDIR)

clean:
	rm -f *.o *.dat *~ $(TESTS)
