Issues/bugs
===========

Reporting
---------

To report a bug in the software click `here <https://github.com/cescalara/minieuso_cpu/issues>`_ to use the GitHub issues facility.

Known issues
------------

A list of known issues that are actively being worked on:

* :strike:`the compatilbility of the current aDIO software with the CMX34BTS CPU model (the problems with switching the LVPS using the new CPU model)`
* :strike:`making the UsbManager class more robust to storage device recognition`
* the digitemp software fails to read the OneWire temperature sensors via an RS-232 adapter (USB is OK)
* incompatibility of variable files with current version of ETOT

  * currently ETOT can only read files with ``N1 = N2 = 4`` and no asychronous acquisition from the thermistors
  * variable file reading library needed


Debugging
---------

When reporting issues with the software there are some debugging tools that you can use to help identify the problem and speed up the process of fixing it.


``gdb`` is a standard debugging tool: https://www.gnu.org/software/gdb/

To run:

1. install if not already installed (``apt-get install gdb``)
2. type ``gdb`` to enter the debug environment, then
3. ``file mecontrol`` to load the binary
4. ``run mecontrol -log ...`` to run the program as required

Some other useful commands:

* ``break <line_number>`` to stop the program at a specific line of ``mecontrol.cpp`` (the main program)
* ``n/<enter>`` to move to the next line of code
* ``s`` to step inside a function
* ``bt`` to display the backtrace (useful after a segmentation fault, for example)
* ``info threads`` get information on exisiting threads
  
For further commands please see the `gdb documentation <https://sourceware.org/gdb/download/onlinedocs/gdb/index.html>`_.
