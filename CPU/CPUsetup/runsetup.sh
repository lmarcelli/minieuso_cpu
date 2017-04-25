#!/bin/bash

#CPU SETUP SCRIPT for Mini-EUSO
#Francesca Capel January 2017
#capel.francesca@gmail.com
#Please see README for correct setup procedure

HOME_DIR="/home/software/CPU"
ZYNQ_IP=192.168.7.10
CPU_IP=192.168.7.2
TEST_IP=8.8.8.8

echo "Mini-EUSO CPU setup"
echo "*******************"

#Turn off the annoying beeps
setterm -blength 0

#Set the timeout of raising the network interface
cp $HOME_DIR/CPUsetup/reduce_timeout.conf /lib/systemd/system/networking.service.d/

#Download the necessary packages
if ping -q -c 1 -W 1 $TEST_IP > /dev/null 2>&1
then
   echo "Connected to internet..."
   echo "Downloading packages..."
   apt-get -y update 
   apt-get -y install build-essential vsftpd expect libraw1394-11 libgtk2.0-0 \
   libgtkmm-2.4-dev libglademm-2.4-dev libgtkglextmm-x11-1.2-dev libusb-1.0-0 \
   stress bridge-utils git-core emacs usbmount
   echo "Packages downloaded"
else
       echo "Could not connect to internet. Exiting..."
       exit 1
fi

#Set up the FTP server and necessary directories
echo "Setting up the FTP server and directories..."
mkdir /home/minieusouser/DATA > /dev/null 2>&1
chown minieusouser /home/minieusouser/DATA
mkdir /home/minieusouser/DONE > /dev/null 2>&1
chown minieusouser /home/minieusouser/DONE 
rm /etc/vsftpd.conf > /dev/null 2>&1
cp $HOME_DIR/CPUsetup/vsftpd.conf /etc/ > /dev/null 2>&1
mkdir /media/usb > /dev/null 2>&1
mkdir /home/minieusouser/log  > /dev/null 2>&1
echo "FTP server is set up"

#Set up the test code and telnet scripts
echo "Compiling the test code..."
mkdir $HOME_DIR/test/bin > /dev/null 2>&1
make -C $HOME_DIR/test/src > /dev/null 2>&1
echo "The test code has been compiled"
chmod +x $HOME_DIR/zynq/telnet/*

#Set up the EM software
mkdir $HOME_DIR/CPUsoftware/log > /dev/null 2>&1
make -C $HOME_DIR/CPU/CPUsoftware/src > /dev/null 2>&1

#Setup symlinks for commands
echo "Creating symlinks"
ln -s $HOME_DIR/zynq/scripts/acqstart_telnet.sh /usr/local/bin/acqstart_telnet
ln -s $HOME_DIR/zynq/scripts/cpu_poll.sh /usr/local/bin/cpu_poll
ln -s $HOME_DIR/zynq/scripts/send_telnet_cmd.sh /usr/local/bin/send_telnet_cmd
ln -s $HOME_DIR/test/bin/test_systems /usr/local/bin/test_systems
echo "Symlinks created"

#Network configuration 
echo "Setting up the network configuration..."
cp $HOME_DIR/CPUsetup/interfaces /etc/network/ > /dev/null 2>&1
echo "Network configuration is set up"

#Set up the cameras 
echo "Setting up the camera software..."
chmod +x $HOME_DIR/cameras/flycapture2-2.3.2.14-amd64/install_flycapture.sh
(cd $HOME_DIR/cameras/flycapture2-2.3.2.14-amd64 && sh install_flycapture.sh)
rm /etc/default/grub
cp $HOME_DIR/CPUsetup/grub /etc/default/
update-grub
#sh -c 'echo 1000 > /sys/module/usbcore/parameters/usbfs_memory_mb'
mkdir $HOME_DIR/cameras/multiplecam/bin > /dev/null 2>\&1
make -C $HOME_DIR/cameras/test/src 
chmod +x $HOME_DIR/cameras/test/multiplecam_test.sh
echo "Camera software is set up"

#Set up the analog board
echo "Setting up the analog board software..."
rmmod rtd520
touch /etc/modprobe.d/blacklist.conf
> /etc/modprobe.d/blacklist.conf 
echo "blacklist rtd520" >> /etc/modprobe.d/blacklist.conf
echo "rtd_dm75xx" >> /etc/modules
make -C $HOME_DIR/analog/driver
(cd $HOME_DIR/analog/driver && make load)
mkdir /lib/modules/$(uname -r)/kernel/rtd/
cp $HOME_DIR/analog/driver/rtd-dm75xx.ko /lib/modules/$(uname -r)/kernel/rtd/
(cd $HOME_DIR/analog/driver && depmod -a)
echo "lsmod | grep rtd:"
lsmod | grep rtd
make -C $HOME_DIR/analog/lib
mkdir $HOME_DIR/analog/bin
make -C $HOME_DIR/analog/src
echo "analog software is set up"

#Set up autologin to root 
echo "Setting up autologin to root user on boot..."
mkdir /etc/systemd/system/getty@tty1.service.d
touch /etc/systemd/system/getty@tty1.service.d/autologin.conf
> /etc/systemd/system/getty@tty1.service.d/autologin.conf
echo "[Service]" >> /etc/systemd/system/getty@tty1.service.d/autologin.conf
echo "ExecStart=" >> /etc/systemd/system/getty@tty1.service.d/autologin.conf
echo "ExecStart=-/sbin/agetty -a root --noclear %I $TERM" >> /etc/systemd/system/getty@tty1.service.d/autologin.conf
echo "OK"


#Press enter to continue
read -p "Press enter to continue, the system will reboot to make changes."

systemctl daemon-reload
#Restart the CPU
reboot









   
   
       
   


