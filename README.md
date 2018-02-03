Mini-EUSO CPU Software
======================

The CPU software controls the startup, data acquisition and housekeeping of the Mini-EUSO instrument. The data acquisition chain of the main instrument is controlled by the Zynq board of the PDM-DP system. The setup of the CPU environment needed to run the CPU software is automated by running the scripts in the ```CPUsetup/``` directory as detailed below.

Further information on the status of the Mini-EUSO integration and testing, as well as more detailed documentation, can be found on the Mini-EUSO wiki page: https://jemeuso.riken.jp/wiki/index.php?Mini-EUSO

Table of Contents
=================

* [Install](#install)
* [Update](#update)
* [SSH connection](#ssh-connection)
* [The software](#the-software)
    * [Usage](#usage)
    * [Functionality](#functionality)
    * [The data format](#the-data-format)
    * [Backwards compatibility](#backwards-compatilbilty)
* [Hardware interfaces](#hardware-interfaces)
    * [Ethernet ports](#ethernet-ports)
    * [aDIO ports - LVPS](#adio-ports-lvps)
    * [DM75xx ports - Photodiodes/SiPMs](#dm75xx-ports)
    * [Thermistors](#thermistors)
    

# Install

1. Set up the CPU hardware with a keyboard, screen and working ethernet connection

2. Download and install the Debian amd64 Standard Desktop from a bootable USB. Follow the default options and install onto the 32 GB flash storage of the CPU.
 * user: minieusouser
 * password: in documentation
 * connect to the network mirror to allow updates
 * install GRUB onto the 32 GB flash drive 

3. Restart the system and boot into the OS, login as root
 * ensure the analog board is also connected (necessary to load the drivers)
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
 
6. Set the local time to UTC 

# Update
To update the software following installation: 

1. Connect to the internet 

2. Run ```git pull``` from the command line within the ```/home/software``` directory

3. Run ```make``` inside ```/home/software/CPU/CPUsoftware```

4. The executable ```mecontrol``` will now be available for use

# SSH connection
Mini-EUSO has 2 ethernet ports, eth0 as a connection to the outside world and eth1 configured for a statuc connection to the Zynq board. eth0 can be used both for connection to the internet and over ssh. Simply check the IP adress of eth0 once connected to your machine and run the following command:
```
ssh minieusouser@<ip_address>
```
Once logged in, run ```su -l``` to run as superuser. 

# The software
The source code is inside the ```CPU/CPUsoftware/``` directory and divided into ```src/```,  ```lib/``` and ```include/```. To build the software run ```make``` inside ```CPUsoftware/```. This will create and symlink the executable ```mecontrol```.

## Usage
To run the software in default mode (standard untriggered data gathering without high voltage) simply run:
```
mecontrol
```

For a full list of command line options run:
```
mecontrol -help
```

To safely stop the software whilst it is running, use ```CTRL-C```. 

## Functionality
* Full power control of the instrument subsystems via the LVPS (low voltage power supply) and the HV (high voltage)
* Data acquisition: 
  * data from the PDM is collected as specified by the command line options, packets are sent from the Zynq every 5.24s with 3 levels of data (D1, D2 and D3) and information on timestamping and the HV status. 
  * The number of packets for D1 and D2 data, N1 and N2, can be set in the configuration file
  * analog and housekeeping data are also gathered every 5.24 s and packaged together with the Zynq data into a CPU packet
  * one CPU packet is appended to the current CPU run file every 5.24s
  * data from the cameras is collected independently with both cameras operating simultaneously
* if no USB storage is available, the output data from the CPU is in ```/home/minieusouser/DONE``` with filenames of the form ```CPU_RUN_<run_type>__<current_date>__<current_time>.dat```
  * S-curve files also have the configured dynode voltage appended to the filename, even if the HV is not switched on 
  * the data format of these files is documented in ```CPUsoftware/src/data_format/data_format.h``` 
  * log files are in ```CPUsoftware/log```, if log output is switched on with ```-log```
* the output data from the cameras is in ```cameras/multiplecam/<NIR/VIS>/<current_date>```
  * .raw for the photos from the cameras
  * log files are in ```cameras/multiplecam/log/```
* if USB storage is detected on the system, the output files will be automatically written there instead, and backed up if there is more than one device.

## The data format
PDM data is acquired, triggered and time-stamped in the Zynq board. This data is then passed to the CPU. The CPU also acquires data from the other subsystems and packages this together with the PDM data during nominal night-time observations. Data is acquired every 5.24 s (128 x 128 x 128 x 1 GTU, 1 GTU = 2.5 us). The CPU generates 2 types of files, CPU_RUN_MAIN containing the standard data acquistion and CPU_RUN_SC for S-curve data. Both data file have a matryoshka structure that is summarised below.

1. The CPU_RUN_MAIN file format
![](CPU/images/cpu_format.png?raw=true)

At present the Zynq data format is fixed so that each ZYNQ_PACKET contains:
* N1 x 128 GTU packet of L1 data (1 byte/pixel)
* N2 x 128 GTU_L2 packet of L2 data (2 bytes/pixel) 
* 1 x 128 GTU_L3 packet of L3 data (4 bytes/pixel)
The data format holds for both triggered and non-triggered readout.

2. The CPU_RUN_SC file format
![](CPU/images/sc_data_format.png?raw=true)

The CPU_RUN_SC has a fixed size which represents the maximum number of threshold steps (0 - 1023). For S-curves taken over a smaller threshold ranges, the file is simply padded with the value 0xFFFFFFFF. S-curve accumulation is calculated on-board the Zynq FPGA using the HLS scurve_adder (https://github.com/cescalara/zynq_ip_hls) allowing for S-curves to be taken with high statistics and stored in a small file size. 

The format is described in detail by the two header files ```pdmdata.h``` (the Zynq data format - depends on the firmware version) and ```data_format.h``` (the CPU data format - depends on the CPU software version).

A 32 bit CRC is calculated for each CPU_RUN file prior to adding the CpuFileTrailer (the last 10 bytes). This CRC is appended to each CPU_RUN file as part of the CpuFileTrailer. 

3. The CPU_RUN_HV file format

This file also has a fixed size and is used to store information on the HV status at the end of a run. This information is additional and complementary to that stored inside the ZYNQ_PACKETS.

## Backwards compatibility
Stable ersions of the software used in previous integration tests of the Mini-EUSO instrument are stored in the following branches named after the integration date. The current stable version of the software is in the ```master``` branch. 

1. ```aug_06_2017```: August integration in Tor Vergata, Rome
2. ```oct_16_2017```: October integration in Tor Vergata, Rome
3. ```dec_17_2017```: December integration in Tor Vergata, Rome

Some key differences of the previous software with the current version:
* ```aug_06_2017```
  * The CPU generates only one run file, with the S-curve packet stored first, followed by RUN_SIZE CPU packets. 
  * S-curves are gathered from DAC 0 - 1000 (inclusive), with a step size of 1 and an accumulation of 1.
  * S-curve accumulation is not calculated, the frames are simply stored for post-processing
* ```oct_16_2017```
  * no LVPS control
  * no variable N1 and N2
* ```dec_16_2017```
   * no automated instrument mode switching
   * no LVPS status checking
   * less robust checks on subsystems
   * different class structure


# Hardware interfaces
The software is designed to cater to a specific hardware setup, with well defined ethernet interfaces and analog and digital I/O channels. These are desribed here.

## Ethernet ports
The CPU system has two ethernet ports: 
1. eth0 - external internet or ssh connection - CN30
2. eth1 - static IP connection to the Zynq board - CN20

The location of these two ports is shown in here:
![](CPU/images/cpu_ethernet_ports.png?raw=true)

**NB: this is true for the CMX34GS model (original CPU), for the CMX34BTS model (new CPU) the ethernet ports are switched.**

## aDIO ports - LVPS
The advanced digital I/O ports (aDIO, CN6) of the CPU are used as an interface to the LVPS, in order to control the power to the instrument subsystems. The Port0 8-bit programmable port is used for handling these commands. The pinout of this connecter is:

| Pin      | I/O Port | Function  | Pin      | I/O Port | Function   | 
| -------- | -------- | --------- | -------- | -------- | ---------- |
| **1**    | P0-0     | HK ON     | **2**    | P0-1     | HK OFF     |
| **3**    | P0-2     | ZYNQ ON   | **4**    | P0-3     | ZYNQ OFF   |
| **5**    | P0-4     | CAM ON    | **6**    | P0-5     | CAM OFF    |
| **7**    | P0-6     |           | **8**    | P0-7     |            |
| **9**    |          | STROBE0   | **10**   |          | STROBE1    |
| **11**   | P1-0     | HK CHECK  | **12**   | P1-1     | ZYNQ CHECK |
| **13**   | P1-2     | CAM CHECK | **14**   | P1-3     |            |
| **15**   |          | DGND      | **16**   |          | +5 V       |


## DM75xx ports - SiPM/photodiodes
The DM75xx series board is used in addition to the main CPU board to handle the analog acquisition. An external 68 pin I/O connector (CN3) is used to interface to the analog signals, but only the utlised channels are shown here. 

| Pin      | Analog channel | Function  | Pin      | Analog channel | Function  | 
| -------- | -------------- | --------- | -------- | -------------- | --------- |
| **1**    | 1              | PH 1.1    | **3**    | 2              | PH 2.1    |
| **5**    | 3              | PH 1.2    | **7**    | 4              | PH 2.2    |
| **11**   | 5              | SiPM 1    | **13**   | 6              | SiPM 64.1 |
| **15**   | 7              | SiPM 64.2 | **17**   | 8              | SiPM 64.1 |
| **9**    |                | AINSENSE  | **10**   |                | AGND      |

## Thermistors
The OneWire thermistors are connected to the CPU via a OneWire-RS232 adapter on the COM1/3 port. They are read out asynchronously and stored in the main CPU file in separate THERM_PACKETs.
