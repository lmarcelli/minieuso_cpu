# Mini-EUSO CPU Software

The CPU software controls the startup, data acquisition and housekeeping of the Mini-EUSO instrument. The data acquisition chain of the main instrument is controlled by the Zynq board of the PDM-DP system and the software can be found in the zynq/ directory. The setup of the CPU environment needed to run the CPU software is automated by running the scripts in the CPUsetup/ directory as detailed below.

# Install

1. Set up the CPU hardware with a keyboard, screen, USB connector and working ethernet connection

2. Download and install the Debian i386 Standard Desktop, following the default options, onto the 32 GB flash storage of the CPU.
  * root password: minieusopass
  * user: minieusouser
  * user password: minieusopass
  * connect to the network mirror to allow updates
  * install GRUB onto the 32 GB flash drive 

3. Restart the system and boot into the OS, login as root

4. Download the CPU/ directory here onto a USB and plug into the CPU

5. Mount the USB and copy over the directory to /home/minieusouser/CPU/

6. Run /home/minieusouser/CPU/CPUsetup/runsetup.sh

