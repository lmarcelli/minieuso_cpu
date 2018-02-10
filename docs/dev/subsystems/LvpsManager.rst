Low voltage power supply (LVPS)
===============================

Description
-----------

The LvpsManager class handles the switching of the separate subsystems on/off by sending a digital pulse from the aDIO port on the CPU board itself (CN6).This pulse must be 5V with 50 mA of current for a duration of 10 ms. In order to provide sufficient current, the CPU aDIO ports are wired through a booster circuit before the connection to the LVPS. The LvpsManager has a set of functions which interface to the aDIO_library in order to send the required pulse to the correct pins on Port0 and read in the return value from the subsystems on Port1. When a subsystem is switched on, a return value of 5V should be read on the correspond input line. The ports for each subsystem are defined in the header file AnalogManager.h. Every time a subsystem is switched on/off, the return lines are checked to verify the expected behaviour. The pinout of the aDIO port is described in the hardware interfaces section of this documentation.


LvpsManager
-----------

.. doxygenclass:: LvpsManager
   :members:
   :private-members:

