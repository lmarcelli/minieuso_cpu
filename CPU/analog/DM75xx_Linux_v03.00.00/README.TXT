Linux Software (DM75XX)

SWP-700010129 rev C

Version v03.00.00.99119

Copyright (C) RTD Embedded Technologies, Inc.  All Rights Reserved.

This software is licensed as described in the RTD End-User Software License
Agreement.  For a copy of this agreement, refer to the file LICENSE.TXT (which
should be included with this software) or contact RTD Embedded Technologies,
Inc.



------------------
Supported Hardware
------------------

This software supports the RTD DM7520, SDM7540 and SDM8540 dataModules.

It should be noted that this driver replaces the following:

	SWP-700010049 rev A     Linux Software (SDM7540)
	SWP-700010005 rev C	Linux Software (DM7520)



-------------------------
Supported Kernel Versions
-------------------------

This software has been tested with the following Linux distributions and kernel
versions:

    * openSUSE 11.4 (unmodified distro kernel 2.6.37)
    * openSUSE 13.2 (unmodified distro kernel 3.16)
    * openSUSE 13.2 (w/kernel 4.2 (or later) from kernel.org)
    

Note that due to kernel API differences between kernel versions, RTD cannot
guarantee compatibility with kernels and distributions not listed above.  It may
be necessary to modify the kernel code and/or Makefiles for your specific Linux
environment.

--------------------------
Supported CPU Architecture
--------------------------
  
 This software has been validated on the following CPU architectures.
 
    * i586 (32-bit) single-core
    * i586 (32-bit) dual-core
    * x86-64 (64-bit) dual-core 
    * x86-64 (64-bit) quad-core 

-------------------
Supported Compilers
-------------------

The driver software was compiled using the GNU gcc compiler provided 
under openSUSE 11.4 and openSUSE 13.2



------
Driver
------

The directory driver/ contains source code related to the driver.


In order to use a driver, one must first compile it, load it into the kernel,
and create device files for the board(s).  To do this, issue the following
commands while sitting in the driver/ directory:


    * make
    * sudo make load


The driver module must be loaded and device files must be created before running
any program which accesses a DM75xx device.



-----------------
Library Interface
-----------------

The directory lib/ contains source code related to the user library.


The DM75xx library is created with a file name of librtd-dm75xx.a and is
statically linked.  These functions are prototyped in the file include/dm75xx_library.h;
this header file must be included in any code which wishes to call library
functions.


The library must be built before compiling the example programs or your
application.


To build the library, issue the command "make" within lib/.



------------
Header Files
------------

The directory include/ contains all header files needed by the driver, example
programs, library, and user applications.



-----------------
Known Limitations
-----------------

 1. This software was tested only on little-endian processors.  If you are using
    a big-endian CPU, you will need to examine the driver, example, and library
    source code for endianness issues and resolve them.
    
 2. This software was written in an attempt to remain architecture independent;
    however, it was only developed and tested on x86 processors.

 2. Many conditions affect board throughput and interrupt performance.  For a
    discussion of these issues, please see the Application Note SWM-640000021
    (Linux Interrupt Performance) available on our web site.

 3. Three of the DMA demand-mode sources (A/D Sample Counter, D/A 1 Sample 
    Counter (and D/A 2 Sample counter) do not operate reliably and are not 
    supported in this driver.

 4. The UTC1 DMA demand-mode source may not work correctly in older firmware 
    versions.  To obtain a newer version of the EPLD firmware contact RTD's 
    technical support.
    
 5. When using High Speed Digital DMA, the amount of data you request must be a 
    multiple of half of the board's FIFO size.  This is because there is no 
    'FIFO Half Full' Demand Mode DMA Request line for that FIFO.
    
 6. Many of the example programs which use DMA are set to a high acquisition 
    rate to show the speed at which speeds the board is capable.  However, for
    slower CPU's (<500 MHz) you may notice buffer overruns.  To correct this
    issue simply reduce the speed at which the programs are acquiring data, or
    increase the buffer size used by the program.

 7. The standard install of openSUSE 11.4 (2.6.37.1-1.2) will not run examples
    that change the scheduling priority of threads (adc_dma, etc), even when
    run as root.  To resolve this, either update/patch the kernel, or execute
    this command after login:
    
    sysctl -w kernel.sched_rt_runtime_us=-1
    
 8. On some distributions of Linux, a driver module may be included that will
    interfere with the use of this module.  If trying to load this driver results
    in a "Device or Resource Busy" error, or in a failure to allocate the I/O memory
    range, then execute an "lsmod" and see if the "rtd520" module is loaded. 
    If so, that module must be removed before this driver module can be inserted.  
    You can remove the module by executing "rmmod rtd520" as root.  To remove the
    module permanently, you'll have to research how to blacklist a module for your 
    particular Linux distro.
    
-------------------------
Getting Technical Support
-------------------------

If you require additional support with this product, or any other products from
RTD Embedded Technologies, contact us using the information below:

	RTD Embedded Technologies, Inc.
	103 Innovation Boulevard
	State College, PA 16803 USA

	Telephone: (814) 234-8087
	Fax: (814) 234-5218
	Sales Information and Quotes: sales@rtd.com
	Technical Assistance: techsupport@rtd.com
	Web Site: http://www.rtd.com
