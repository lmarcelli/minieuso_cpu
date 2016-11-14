#!/bin/bash

#CPU SETUP SCRIPT for Mini-EUSO
#Francesca Capel November 2016
#capel.francesca@gmail.com
#Please see README for correct setup procedure

ZYNQ_IP=192.168.7.10
CPU_IP=192.168.7.2

echo "Mini-EUSO CPU setup"
echo "*******************"

#Download the necessary packages
if ping -q -c 1 -W 1 $TEST_IP > /dev/null 2>&1
then
   echo "Connected to internet..."
   echo "Downloading packages..."
   apt-get update && upgrade > /dev/null 2>&1
   apt-get install build-essential vsftpd expect > /dev/null 2>&1
else
       echo "Could not connect to internet. Exiting.."
       exit 1
fi

#Setup the FTP server
echo "Setting up the FTP server..."
mkdir /home/ftp-home > /dev/null 2>&1
./create_ftpuser.sh
sleep 10

#Setup symlinks for commands
echo "Creating symlinks"
mv -r /home/CPU/zynq/scripts /home/ftp-home/
ln -s /home/ftp-home/scripts/acqstart_telnet.sh /usr/local/bin/acqstart_telnet
ln -s /home/ftp-home/scripts/cpu_poll.sh /usr/local/bin/cpu_poll
ln -s /home/ftp-home/scripts/send_telnet_cmd.sh /usr/local/bin/send_telnet_cmd

#Network configuration
echo "Setting up the network configuration..."
echo "/n" > /etc/network/interfaces
echo "auto eth0" > /etc/network/interfaces
echo "iface eth0 inet static" > /etc/network/interfaces
echo "\taddress 192.168.7.2" > /etc/network/interfaces
echo "\tnetmask 255.255.255.0" > /etc/network/interfaces
echo "\tgateway 192.168.7.254" > /etc/network/interfaces

#Setup autologin to root 
echo "Setting up autologin to root user on boot..."
systemctl set-default multi-user.target
mkdir /etc/systemd/system/getty@tty1.service.d
touch /etc/systemd/getty@tty1.service.d/autologin.conf
echo "[Service]" > /etc/systemd/getty@tty1.service.d/autologin.conf
echo "ExecSart=" > /etc/systemd/getty@tty1.service.d/autologin.conf
echo "ExecStart=-/sbin/agety -a root --noclear %I $TERM" > /etc/systemd/getty@tty1.service.d/autologin.conf
systemctl daemon-reload
systemctl restart getty@tty1.service

#Reboot
echo "Rebooting..."
sleep 3
reboot








   
   
       
   


