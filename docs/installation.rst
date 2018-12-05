
Installation
============

Requirements
------------

The Mini-EUSO software is designed to run with specific hardware configuration making use of both COTS products and custom electronics boards developed by members of the EUSO collaboration.

**NB: If you wish to compile and run the software on any normal laptop/computer without the aforementioned specific hardware configuration, please see** `these instructions <https://minieuso-software.readthedocs.io/en/latest/source_code.html>`_. 

The software is designed to run on PC/104 CMX34GS or CMX34BT single board CPU (https://www.rtd.com/PC104/CM/CMX34BT/CMX34BT.htm) but will compile on any linux-based operating system (tested on Ubuntu 16.04 and Debian 8). This board is used in conjunction with the DM35520HR 12 bit analog I/O module connected to the main CPU board via the standard PC1Ie/104 bus.    

In Debian 8, the required packages are::

  build-essential vsftpd expect libraw1394-11 libgtk2.0-0 \
  libgtkmm-2.4-dev libglademm-2.4-dev libgtkglextmm-x11-1.2-dev \
  libusb-1.0-0 libusb-1.0 stress bridge-utils \
  git-core emacs usbmount gdb ntp libboost-all-dev \
  inotify-tools digitemp libasan lsb_release linux-headers-3.16.0-6-amd64 

The setup script, ``CPU/CPUsetup/run_setup.sh`` takes care of the installation of these packages, as detailed below.

Installation instructions
-------------------------

1. Set up the CPU hardware with a keyboard, screen and working ethernet connection

2. Download and install the Debian amd64 Standard Desktop from a bootable USB. Follow the default options and install onto the 32 GB flash storage of the CPU.

   * user: minieusouser
   * password: on the EUSO wiki (members only)
   * connect to the network mirror to allow updates
   * install GRUB onto the 32 GB flash drive 

3. Restart the system and boot into the OS, **login as root** using the same password and ``su -l``

   * ensure the **analog board** and **cameras** are also connected (necessary to load the drivers)
   * the ethernet connection can be configured by copying the following lines into the ``/etc/network/interfaces`` file
   * the CPU has 2 ports (eth0 and eth1), so edit this file as necessary::

       auto eth0
       allow-hotplug eth0
       iface eth0 inet dhcp
  
   * following this run ``service networking restart`` from the command line

4. Download the software from the repository::
     
     apt-get install git-core
     git clone https://github.com/cescalara/minieuso_cpu /home/software

5. Run the setup script::
     
     cd /home/software/CPU/CPUsetup/
     ./runsetup.sh 

* downloads the necessary packages
* sets up the FTP server
* builds the software
* set the local time to UTC 
* configures the network for use with the Zynq board
* sets up autologin to the root user on boot
* restarts the shell 

**NB: this script should only be run once, when first setting up the CPU, if you need to clone the git repository again, see the Update section of these docs**
  
6. Check the time on the CPU is sensible (it resets if the battery is disconnected) using ``date``

* if the time is incorrect, use, for example::
       
    hwclock --set --date “2017-10-04 09:41:50” --utc
    hwclock --hctosys
       
to set the current time.
