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
mecontrol -db -log -hv -long -trig
```

* db: runs in debug mode (test functionallity executed)
* log: produces a log with all levels of output printed
* hv: switches the high voltage on to normal operational level (1100 V)
* long: takes a long acquisition (i.e. until interrupted)
* trig: runs with triggered data acquisition

## Functionality
* the data acquisition: 
  * data from the PDM is collected as specified by the command line options, packets are sent from the Zynq every 5.24s with 3 levels of data and information on timestamping and the HV status. Level 1 and Level 2 have 4 packets of data and level 3 has 1.
  * analog and housekeeping data is also gathered every 5.24 s and packaged together with the Zynq data into a CPU packet
  * one CPU packet is appended to the current CPU run file every 5.24s
  * data from the cameras is collected by acquiring with one camera at a time,  waiting 5.24s between acquisitions
* the output data from the CPU is in ```/home/minieusouser/DONE``` with filenames ```CPU_RUN__<current_date>__<current_time>.dat```
  * the data format of these files is documented in ```CPUsoftware/include/data_format.h``` 
  * log files are in ```/home/minieusouser/log/```
* the output data from the cameras is in ```cameras/multiplecam/<current_date>```
  * .png for the photos from the cameras
  * log files are in ```cameras/multiplecam/log/```

## Backwards compatibility
The software is designed to be compatible with previous versions of the Zynq board firmware used during the integration and testing of the Mini-EUSO engineering model. Most of the compaitibilty is taken care of automatically. However in order to use the original "single event" readout used during testing from Jan - Aug 2017, it is necessary to follow these steps:

1. In ```CPUsoftware/include/data_format.h``` uncomment L16 ```#define SINGLE_EVENT```
2. Recompile the code by running ```make``` in ```CPUsoftware/src```

