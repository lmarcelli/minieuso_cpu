Code organisation
=================

The software is stored in ``CPUsoftware/`` with the following file structure:

The source code: 

* ``src/`` : main source code
* ``inc/`` : include headers
* ``lib/`` : libraries required to run the software

Build dirs:

* ``obj/`` : objects files produced on make
* ``bin/`` : executable

Configuration and logs:

* ``log/`` :  log files written here
* ``config/`` : configuration files sotred here

**NB: The data format headers are sotred outside the main code as they need to be accessed by other software** 
The data format headers are stored in the highlest level directory of the GitHub repository

* ``data_format/`` : the header files describing the data format from Zynq and the CPU

  * ``data_format.h`` - CPU data files
  * ``pdmdata.h`` - structures defined in the Zynq board software


The code can be compiled using the Makefile by running ``make`` in ``CPUsoftware/``. First, the libraries must be compiled by running ``make`` in ``CPUsoftware/lib``.

When compiled, the software produces the mecontrol executable which is symlinked to ``/usr/local/bin`` by the Makefile. This command runs the ``mecontrol.cpp`` program and accepts a wide range of command line arguments, which are detailed in the usage documentation, and can also be checked by running ``mecontrol -help``.

In more detail
--------------

Inside the ``src/`` directory, the main source code is organised by functionality as follows:

* ``main/`` : the main program

  * ``mecontrol.cpp``
  * ``mecontrol.h``

* ``instrument/`` : the RunInstrument class to control the whole instrument

  * ``RunInstrument.cpp`` - control of instrument and mode switching
  * ``RunInstrument.h``

* ``operations/`` : OperationMode base class and two daughter classes

  * ``OperationMode.cpp`` - base class for an operational mode
  * ``OperationMode.h``
  * ``DataAcquisition.cpp`` - data acquisition (NIGHT mode)
  * ``DataAcquisition.h``
  * ``DataReduction.cpp`` - data reduction (DAY mode)
  * ``DataReduction.h``

* ``subsystems/`` : Manager classes to control all the necessary subsystems

  * ``AnalogManager.cpp`` - analog acquisition using the DM75xx board 
  * ``AnalogManager.h``  (photodiodes, SiPMs)
  * ``CamManager.cpp`` -  interface to the camera software
  * ``CamManager.h``
  * ``LvpsManager.cpp`` - powering of subsystems using the LVPS
  * ``LvpsManager.h``
  * ``ThermManager.cpp`` - thermistor acquisition 
  * ``ThermManager.h``
  * ``UsbManager.cpp`` - usb storage and data backup 
  * ``UsbManager.h``
  * ``ZynqManager.cpp`` - Zynq board interface via telnet
  * ``ZynqManager.h``
    
* ``tools/`` : useful tools used throughout the software

  * ``ConfigManager.cpp`` - parsing the configuration file
  * ``ConfigManager.h`` 
  * ``CpuTools.cpp`` - useful functions
  * ``CpuTools.h``
  * ``InputParser.cpp`` - parsing command line input
  * ``InputParser.h``
  * ``SynchronisedFile.cpp`` - safe asynchronous file writing
  * ``SynchronisedFile.h``
  * ``log.cpp`` - logging
  * ``log.h``


This is just intended to give an overview and further details are provided in the class documentation in the :doc:`development <development.rst>` section. 
