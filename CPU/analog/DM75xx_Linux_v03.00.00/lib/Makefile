#
#	FILE NAME: Makefile
#
#	FILE DESCRIPTION: Make description file for building user library
#
#	PROJECT NAME: Linux Software (DM75xx)
#
#	PROJECT VERSION: (Defined in README.TXT)
#
#	This file and its contents are copyright (C) RTD Embedded Technologies,
#	Inc.  All Rights Reserved.
#
#	This software is licensed as described in the RTD End-User Software
#	License Agreement.  For a copy of this agreement, refer to the file
#	LICENSE.TXT (which should be included with this software) or contact RTD
#	Embedded Technologies, Inc.
#


CC=gcc
DEBUG_FLAGS=-g
INCLUDE_FLAGS=-I../include
OPTIMIZE_FLAGS=-O6
WARNING_FLAGS=-Wall
CFLAGS:=$(DEBUG_FLAGS) $(INCLUDE_FLAGS) $(OPTIMIZE_FLAGS) $(WARNING_FLAGS)

all:			librtd-dm75xx.a

librtd-dm75xx.a:	librtd-dm75xx.o
	ar -cru librtd-dm75xx.a librtd-dm75xx.o

librtd-dm75xx.o:	librtd-dm75xx.c
	$(CC) $(CFLAGS) -c librtd-dm75xx.c

clean:
	rm -f *.o *.a *~
