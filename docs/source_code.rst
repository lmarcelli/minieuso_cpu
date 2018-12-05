Source code/compilation
=======================

The source code for this project can be found on GitHub: https://github.com/cescalara/minieuso_cpu

The source code should compile on any unix-based system and has been tested on linux (Debian 8, Ubuntu 16) and macOS (High Sierra). Obviously, for full functionality of the software, the relevant hardware is required (Zynq board, ancillary detectors etc.), but for certain testing and development these could be replaced with simple hardware simulators.

Compilation
-----------

1. To compile the CPU software, first install git tools on your system and clone the repository as follows::

     git clone https://github.com/cescalara/minieuso_cpu /my/desired/location

Alternatively, the software can be downloaded by visiting `this <https://github.com/cescalara/minieuso_cpu>`_ page and clicking on the green *clone or download* button. 

2. Once you have a copy of the software, install the necessary packages. As an example for linux, Debian 8, the following packages are required::

     apt-get install libusb-1.0 inotify-tools libboost-all-dev

3. Before compiling the main software, it is necessary to compile the libraries::

     cd /my/desired/location/CPU/CPUsoftware/lib
     make

4. Now the main software can be compiled::

     cd /my/desired/location/CPU/CPUsoftware/src
     make

If any problems at this stage, please feel free to contact me or open an `issue on GitHub <https://github.com/cescalara/minieuso_cpu/issues>`_. Most compilation errors are due to missing packages and can be solved with a quick google search, so please try this first! All commits to the main repository are required to pass continuous integration checks including a full build of the software on Debian 8. The status of these checks is indicated by a badge on the main repository page.  
