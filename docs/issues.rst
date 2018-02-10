Issues/bugs
===========

Reporting
---------

To report a bug in the software click `here <https://github.com/cescalara/minieuso_cpu/issues>`_ to use the GitHub issues facility.

Known issues
------------

A list of known issues that are actively being worked on:

* the compatilbility of the current aDIO software with the CMX34BTS CPU model (the problems with switching the LVPS using the new CPU model)
* making the UsbManager class more robust to storage device recognition
* incompatibility of variable files with current version of ETOT

  * currently ETOT can only read files with ``N1 = N2 = 4`` and no asychronous acquisition from the thermistors
  * variable file reading library needed
