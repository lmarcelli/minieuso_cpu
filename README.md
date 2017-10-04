# Mini-EUSO CPU Software

The CPU software controls the startup, data acquisition and housekeeping of the Mini-EUSO instrument. The data acquisition chain of the main instrument is controlled by the Zynq board of the PDM-DP system. The setup of the CPU environment needed to run the CPU software is automated by running the scripts in the ```CPUsetup/``` directory as detailed below.

Further information on the status of the Mini-EUSO integration and testing, as well as more detailed documentation, can be found on the Mini-EUSO wiki page: http://jemeuso2.riken.jp/TAEUSO/wiki/index.php?Mini-EUSO

# Install

1. Set up the CPU hardware with a keyboard, screen and working ethernet connection

2. Download and install the Debian amd64 Standard Desktop from a bootable USB. Follow the default options and install onto the 32 GB flash storage of the CPU.
 * user: minieusouser
 * password: in documentation
 * connect to the network mirror to allow updates
 * install GRUB onto the 32 GB flash drive 

3. Restart the system and boot into the OS, login as root
 * the ethernet connection can be configured by copying the following lines into the ```/etc/network/interfaces``` file
 * the CPU has 2 ports (eth0 and eth1), so adjust as necessary.
```
auto eth0
allow-hotplug eth0
iface eth0 inet dhcp
```  
 * following this run ```service networking restart``` from the command line

4. Download the software from the repository
```
apt-get install git-core
git clone https://github.com/cescalara/minieuso_cpu /home/software
```

5. Run the setup script
```
cd /home/software/CPU/CPUsetup/
./runsetup.sh 
```
 * downloads the necessary packages
 * sets up the FTP server
 * sets up the directory structure
 * configures the network for use with the Zynq board
 * installs and sets up the test software for all systems
 * sets up autologin to the root user on boot
 * restarts the shell 

## Update
To update the software following installation: 

1. Connect to the internet 

2. Run ```git pull``` from the command line within the ```/home/software``` directory

## SSH connection
Mini-EUSO has 2 ethernet ports, eth0 as a connection to the outside world and eth1 configured for a statuc connection to the Zynq board. eth0 can be used both for connection to the internet and over ssh. Simply check the IP adress of eth0 once connected to your machine and run the following command:
```
ssh minieusouser@<ip_adress>
```
Once logged in, run ```su -l``` to run as superuser. 

# The software
The source code is inside the ```CPUsoftware/``` directory and divided into ```src/```,  ```lib/``` and ```include/```. To build the software run ```make``` inside ```CPUsoftware/src```. This will create the executable ```mecontrol``` in ```CPUsoftware/bin```.

To run the software in default mode (standard untriggered data gathering without high voltage) simply run:
```
mecontrol
```

The following command line options are available:
```
mecontrol -db -log -hv -long -trig -cam
```

* db: runs in debug mode (test functionallity executed)
* log: produces a log with all levels of output printed
* hv: switches the high voltage on to normal operational level (1100 V)
* long: takes a long acquisition (i.e. until interrupted)
* trig: runs with triggered data acquisition
* cam: runs 2 min camera acquisition for every CPU_RUN_MAIN

## Functionality
* the data acquisition: 
  * data from the PDM is collected as specified by the command line options, packets are sent from the Zynq every 5.24s with 3 levels of data and information on timestamping and the HV status. Level 1 and Level 2 have 4 packets of data and level 3 has 1.
  * analog and housekeeping data is also gathered every 5.24 s and packaged together with the Zynq data into a CPU packet
  * one CPU packet is appended to the current CPU run file every 5.24s
  * data from the cameras is collected by acquiring with one camera at a time,  waiting 5.24s between acquisitions
* the output data from the CPU is in ```/home/minieusouser/DONE``` with filenames ```CPU_RUN__<current_date>__<current_time>.dat```
  * the data format of these files is documented in ```CPUsoftware/include/data_format.h``` 
  * log files are in ```/home/minieusouser/log/```
* the output data from the cameras is in ```cameras/multiplecam/<NIR/VIS>/<current_date>```
  * .raw for the photos from the cameras
  * log files are in ```cameras/multiplecam/log/```

## The data format
PDM data is acquired, triggered and time-stamped in the Zynq board. This data is then passed to the CPU. The CPU also acquires data from the other subsystems and packages this together with the PDM data during nominal night-time observations. Data is acquired every 5.24 s (128 x 128 x 128 x 1 GTU, 1 GTU = 2.5 us). The CPU generates 2 types of files, CPU_RUN_MAIN containing the standard data acquistion and CPU_RUN_SC for S-curve data. Both data file have a matryoshka structure that is summarised below.

1. The CPU_RUN_MAIN file format
![](CPU/images/cpu_format.png?raw=true)

At present the Zynq data format is fixed so that each ZYNQ_PACKET contains:
* 4 x 128 GTU packet of L1 data (1 byte/pixel)
* 4 x 128 GTU_L2 packet of L2 data (2 bytes/pixel) 
* 1 x 128 GTU_L3 packet of L3 data (4 bytes/pixel)
The data format holds for both triggered and non-triggered readout.

2. The CPU_RUN_SC file format
![](CPU/images/sc_data_format.png?raw=true)

The CPU_RUN_SC has a fixed size which represents the maximum number of threshold steps (0 - 1023). For S-curves taken over a smaller threshold ranges, the file is simply padded with the value 0xFFFFFFFF. S-curve accumulation is calculated on-board the Zynq FPGA using the HLS scurve_adder (https://github.com/cescalara/zynq_ip_hls) allowing for S-curves to be taken with high statistics and stored in a small file size. 

The format is described in detail by the two header files ```pdmdata.h``` (the Zynq data format - depends on the firmware version) and ```data_format.h``` (the CPU data format - depends on the CPU software version).

A 32 bit CRC is calculated for each CPU_RUN file prior to adding the CpuFileTrailer (the last 10 bytes). This CRC is appended to each CPU_RUN file as part of the CpuFileTrailer. 

## Backwards compatibility
The software is designed to be compatible with previous versions of the Zynq board firmware used during the integration and testing of the Mini-EUSO engineering model. Most of the compaitibilty is taken care of automatically. However in order to use the original "single event" readout used during testing from Jan - Aug 2017, it is necessary to follow these steps:

1. In ```CPUsoftware/include/data_format.h``` uncomment L16 ```#define SINGLE_EVENT```
2. Recompile the code by running ```make``` in ```CPUsoftware/src```

Some key differences of the old mode of operation relative to the current version:
* The CPU generates only one run file, with the S-curve packet stored first, followed by RUN_SIZE CPU packets. 
* S-curves are gathered from DAC 0 - 1000 (inclusive), with a step size of 1 and an accumulation of 1.
* S-curve accumulation is not calculated, the frames are simply stored for post-processing
