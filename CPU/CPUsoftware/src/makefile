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
LIB_FLAGS:=-L../lib -lrtd-dm75xx -lm -lpthread -lusb-1.0
CFLAGS:=-std=c++11 $(INCLUDE_FLAGS) $(OPTIMIZE_FLAGS) $(WARNING_FLAGS) $(DEBUG_FLAGS)
endif
SOURCES := $(shell find . -name '*.cpp')

OUTDIR = ../bin/

TESTS = \
	mecontrol \

all:    $(TESTS)

mecontrol: mecontrol.cpp
	$(CXX) $(CFLAGS) $(SOURCES) $(LIB_FLAGS) -o mecontrol 
	mv mecontrol $(OUTDIR)

clean:
	rm -f *.o *.dat *~ $(TESTS)
