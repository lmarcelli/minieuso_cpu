#
#	FILE NAME: Makefile
#
#	FILE DESCRIPTION: Make description file for building driver
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
#


MY_CFLAGS := -I$(shell pwd)/../../CPUsoftware/include

#
# Kernel build environment directory.  Supposedly it is safer to use this
# method of referring to it than using /usr/src.
#

KERNEL_DIR := /lib/modules/$(shell uname -r)/build
RTD520_EXISTS := $(shell lsmod | grep rtd520 1>/dev/null 2>&1 && echo "yes" || echo "no")

obj-m := rtd-dm75xx.o

DRIVER_MODULE=rtd-dm75xx.ko
FILES_TO_CLEAN= \
	*.o \
	rtd-dm75xx.ko \
	.*.cmd \
	*.ko \
	.tmp_versions \
	rtd-dm75xx.mod.c \
	.*.d \
	*.symvers \
	*.order \
	*~

driver:		rtd-dm75xx.c
	make -C $(KERNEL_DIR) SUBDIRS=`pwd` EXTRA_CFLAGS="$(MY_CFLAGS)" modules

.PHONY: debug
debug:
	make -C $(KERNEL_DIR) SUBDIRS=`pwd` EXTRA_CFLAGS="$(MY_CFLAGS) -DDM75XX_DEBUG -DDEBUG" modules


clean:
	rm -rf $(FILES_TO_CLEAN)

load:
ifeq ($(RTD520_EXISTS),yes)
	@echo ""
	@echo ""
	@echo "Could not load driver because rtd520 is already loaded."; 
	@echo "Please review README.TXT Known Limitations for resolving"; 
	@echo "this issue."; 
	exit 1;
else
	/sbin/insmod ./$(DRIVER_MODULE); 
	chmod 666 /dev/rtd-dm75xx-*;
endif

unload:
	/sbin/rmmod rtd-dm75xx
	@echo "Removing old device files ..."; \
	rm -f /dev/rtd-dm75xx-* \
	echo ""; \
