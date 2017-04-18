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
git clone https://github.com/cescalara/MiniEUSO /home/software
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
 * sets up autlogin to the root user on boot
 * restarts the shell 

# Update
To Update the software following installation: 

1. Connect to the internet 

2. Run ```git pull``` from the command line within the ```/home/software``` directory

# Run system tests
1. Use the following command to test the simultaneous aquisition from the PDM (via the Zynq board), NIR and visible cameras (via USB) and the photodiode sensors (via the analog board). 
```
test_systems 
```
* the data acquisition 
  * data from the PDM is collected in a non-triggered way, packets are sent from the Zynq every 5.24s with 3 levels of data and information on timestamping and the HV status
  * data from the cameras is collected by acquiring with one camera at a time,  waiting 5s between acquisitions
  * data from the photodiodes is read out into a FIFO and collected every 5s
* the output data from the PDM and photodiodes is in ```/home/minieusouser/DATA/```
  * ```frm_XXXXXXXX.dat``` for the PDM frames
  * ```outputX.dat``` for the photodiode reading
  * log files are in ```/home/minieusouser/log/```
* the output data from the cameras is in ```/home/software/CPU/cameras/multiplecam/<current_date>```
  * .png for the photos from the cameras
  * log files are in ```/home/software/CPU/cameras/multiplecam/log/```

