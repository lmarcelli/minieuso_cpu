#!/bin/bash

#CPU SETUP SCRIPT for Mini-EUSO
#Francesca Capel November 2016
#capel.francesca@gmail.com
#Please see README for correct setup procedure

ZYNQ_IP=192.168.7.10
CPU_IP=192.168.7.2
TEST_IP=8.8.8.8

echo "Mini-EUSO CPU setup"
echo "*******************"

#Download the necessary packages
if ping -q -c 1 -W 1 $TEST_IP > /dev/null 2>&1
then
   echo "Connected to internet..."
   echo "Downloading packages..."
   apt-get -y update > /dev/null 2>&1
   apt-get -y install build-essential vsftpd expect libraw1394-11 libgtk2.0-0 \
   libgtkmm-2.4-dev libglademm-2.4-dev libgtkglextmm-x11-1.2-dev libusb-1.0-0 \
   stress > /dev/null 2>&1
   echo "OK"
else
       echo "Could not connect to internet. Exiting..."
       exit 1
fi

#Setup the FTP server
echo "Setting up the FTP server..."
mkdir /home/minieusouser/DATA > /dev/null 2>&1
chown minieusouser /home/minieusouser/DATA
mkdir /home/minieusouser/DONE > /dev/null 2>&1
chown minieusouser /home/minieusouser/DONE 
rm /etc/vsftpd.conf > /dev/null 2>&1
cp /home/minieusouser/CPU/CPUsetup/vsftpd.conf /etc/ > /dev/null 2>&1
mkdir /media/usb > /dev/null 2>&1
echo "OK"

#Setup symlinks for commands
echo "Creating symlinks"
ln -s /home/minieusouser/CPU/zynq/scripts/acqstart_telnet.sh /usr/local/bin/acqstart_telnet
ln -s /home/minieusouser/CPU/zynq/scripts/cpu_poll.sh /usr/local/bin/cpu_poll
ln -s /home/minieusouser/zynq/scripts/send_telnet_cmd.sh /usr/local/bin/send_telnet_cmd
ln -s /home/minieusouser/test/bin/test_systems /usr/local/bin/test_systems
echo "OK"

#Network configuration (need to comment the previous setup)
echo "Setting up the network configuration..."
cp /home/minieusouser/CPU/CPUsetup/interfaces /etc/network/ > /dev/null 2>&1
echo "OK"

#Setup the cameras 
echo "Setting up the camera software..."
sh /home/minieusouser/CPU/cameras/flycapture2-2.3.2.14-i386/install_flycapture.sh
sh -c 'echo 1000 > /sys/module/usbcore/parameters/usbfs_memory_mb'
echo "cat /sys/module/usbcore/parameters/usbfs_memory_mb: "
cat /sys/module/usbcore/parameters/usbfs_memory_mb
echo "OK"

#Setup the analog board
echo "Setting up the analog board software..."
touch /etc/modprobe.d/blacklist.conf
> /etc/modprobe.d/blacklist.conf 
echo "blacklist rtd520" >> /etc/modprobe.d/blacklist.conf
echo "rtd_dm75xx" >> /etc/modules
make -C /home/minieusouser/CPU/analog/driver
(cd /home/minieusouser/CPU/analog/driver && make load)
cp /home/minieusouser/CPU/analog/driver/rtd-dm75xx.ko /lib/modules/3.16.0-4-686-pae/kernel/rtd/
(cd /home/minieusouser/CPU/analog/driver && depmod -a)
echo "lsmod | grep rtd:"
lsmod | grep rtd
echo "OK"


#Press enter to continue
read -p "Press enter to continue"

#Setup autologin to root 
echo "Setting up autologin to root user on boot..."
mkdir /etc/systemd/system/getty@tty1.service.d
touch /etc/systemd/system/getty@tty1.service.d/autologin.conf
> /etc/systemd/system/getty@tty1.service.d/autologin.conf
echo "[Service]" >> /etc/systemd/system/getty@tty1.service.d/autologin.conf
echo "ExecStart=" >> /etc/systemd/system/getty@tty1.service.d/autologin.conf
echo "ExecStart=-/sbin/agetty -a root --noclear %I $TERM" >> /etc/systemd/system/getty@tty1.service.d/autologin.conf
systemctl daemon-reload
echo "OK"

#Restart the terminal
systemctl restart getty@tty1.service








   
   
       
   


