#
#	FILE NAME: Makefile
#
#	FILE DESCRIPTION: Make description file for building example programs
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
LIBRARY_FLAGS=-L../lib -lrtd-dm75xx  -lm -lpthread
OPTIMIZE_FLAGS=-O6
WARNING_FLAGS=-Wall
CFLAGS:=$(DEBUG_FLAGS) $(INCLUDE_FLAGS) $(OPTIMIZE_FLAGS) $(WARNING_FLAGS)

EXAMPLES = \
    about_intrpt \
    adc_abrst \
    adc_dac_simul \
    adc_dma \
    adc_dma_continuous \
    adc_hd_simul \
    adc_single \
    adc_soft_trig \
    adc_multi \
    adc_trig_ext \
    adio_event \
    adio_match \
    analog_dio \
    calibrate \
    cgt_reset_intrpt \
    dac \
    dac_dma \
    delay_intrpt \
    dma_pci_arb \
    etrig_intrpt \
    hd \
    hd_dma \
    temperature \
    timer \
    timer_intrpt

TESTS = \
    library_test \

all:	$(EXAMPLES) $(TESTS)

about_intrpt: about_intrpt.c
	$(CC) $(CFLAGS) -o about_intrpt about_intrpt.c $(LIBRARY_FLAGS)

adc_abrst: adc_abrst.c
	$(CC) $(CFLAGS) -o adc_abrst adc_abrst.c $(LIBRARY_FLAGS)
    
adc_dac_simul: adc_dac_simul.c
	$(CC) $(CFLAGS) -o adc_dac_simul adc_dac_simul.c $(LIBRARY_FLAGS)
    
adc_dma: adc_dma.c
	$(CC) $(CFLAGS) -o adc_dma adc_dma.c $(LIBRARY_FLAGS)
    
adc_dma_continuous: adc_dma_continuous.c
	$(CC) $(CFLAGS) -o adc_dma_continuous adc_dma_continuous.c $(LIBRARY_FLAGS)

adc_hd_simul: adc_hd_simul.c
	$(CC) $(CFLAGS) -o adc_hd_simul adc_hd_simul.c $(LIBRARY_FLAGS)

adc_single: adc_single.c
	$(CC) $(CFLAGS) -o adc_single adc_single.c $(LIBRARY_FLAGS)

adc_soft_trig: adc_soft_trig.c
	$(CC) $(CFLAGS) -o adc_soft_trig adc_soft_trig.c $(LIBRARY_FLAGS)
    
adc_multi: adc_multi.c
	$(CC) $(CFLAGS) -o adc_multi adc_multi.c $(LIBRARY_FLAGS)
    
adc_trig_ext: adc_trig_ext.c
	$(CC) $(CFLAGS) -o adc_trig_ext adc_trig_ext.c $(LIBRARY_FLAGS)
    
adio_event: adio_event.c
	$(CC) $(CFLAGS) -o adio_event adio_event.c $(LIBRARY_FLAGS)
    
adio_match: adio_match.c
	$(CC) $(CFLAGS) -o adio_match adio_match.c $(LIBRARY_FLAGS)
    
analog_dio: analog_dio.c
	$(CC) $(CFLAGS) -o analog_dio analog_dio.c $(LIBRARY_FLAGS)

calibrate: calibrate.c
	$(CC) $(CFLAGS) -o calibrate calibrate.c $(LIBRARY_FLAGS) -lrt

cgt_reset_intrpt: cgt_reset_intrpt.c
	$(CC) $(CFLAGS) -o cgt_reset_intrpt cgt_reset_intrpt.c $(LIBRARY_FLAGS)
	
dac: dac.c
	$(CC) $(CFLAGS) -o dac dac.c $(LIBRARY_FLAGS)
    
dac_dma: dac_dma.c
	$(CC) $(CFLAGS) -o dac_dma dac_dma.c $(LIBRARY_FLAGS)

delay_intrpt: delay_intrpt.c
	$(CC) $(CFLAGS) -o delay_intrpt delay_intrpt.c $(LIBRARY_FLAGS)
    
dma_pci_arb: dma_pci_arb.c
	$(CC) $(CFLAGS) -o dma_pci_arb dma_pci_arb.c $(LIBRARY_FLAGS)
    
etrig_intrpt: etrig_intrpt.c
	$(CC) $(CFLAGS) -o etrig_intrpt etrig_intrpt.c $(LIBRARY_FLAGS)

hd: hd.c
	$(CC) $(CFLAGS) -o hd hd.c $(LIBRARY_FLAGS)

hd_dma: hd_dma.c
	$(CC) $(CFLAGS) -o hd_dma hd_dma.c $(LIBRARY_FLAGS)
    
temperature:	temperature.c
	$(CC) $(CFLAGS) -o temperature temperature.c $(LIBRARY_FLAGS)
    
timer:	timer.c
	$(CC) $(CFLAGS) -o timer timer.c $(LIBRARY_FLAGS)
    
timer_intrpt:	timer_intrpt.c
	$(CC) $(CFLAGS) -o timer_intrpt timer_intrpt.c $(LIBRARY_FLAGS)

library_test: library_test.c
	$(CC) $(CFLAGS) -o library_test library_test.c $(LIBRARY_FLAGS)

clean:
	rm -f *.o *.dat *~ $(EXAMPLES) $(TESTS)
