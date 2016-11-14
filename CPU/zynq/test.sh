#!/bin/bash

ZYNQ_IP=192.168.7.10
#MAC_IP=172.20.10.5

#add date and time to log files
adddate() {
	while IFS= read -r line; do 
		echo "$(date) $line"
	done
}

#clear log files
> /var/log/testlog

echo "Mini-EUSO CPU testing" | adddate >> /var/log/testlog
echo "**************************************************" >> /var/log/testlog


#Wait for connection to network using ping
until ping -q -c 1 -W 1 $ZYNQ_IP > /dev/null 2>&1;
do
	sleep 1
done
echo "Connection Successful" | adddate >> /var/log/testlog


#Setup FTP server
/etc/init.d/vsftpd stop > /dev/null 2>&1
/etc/init.d/vsftpd start > /dev/null 2>&1
if [ $? -eq 0 ]
 then
        echo "FTP server running" | adddate >> /var/log/testlog
else
        echo "FTP server failed" | adddate >> /var/log/testlog
	exit 1
fi

#Wait for zynq telnet sever to be up to check zynq is ready
until nc -w 5 -z $ZYNQ_IP 23;
do
	echo "unconnected" | adddate >> /var/log/testlog
done
echo "Telnet server up" | adddate >> /var/log/testlog

#Start looking for data
cpu_poll &

#DATA ACQUISITION
i="0"
while [ $i -lt 5 ]
do
	#Send commnad to Zynq to acquire data
 	send_telnet_cmd > /dev/null 2>&1

	#sleep for a certain time and then repeat
	interval=10
	sleep ${interval}
	i=$[$i+1]
done

#kill  cpu_poll
kill -9 `pgrep -f cpu_poll`
echo "Finished data acquisition" | adddate >> /var/log/testlog

#Copy data to disk
mount /dev/sdb2 /media/usb
directory="/home/ftp-home/minieusouser/DONE/"
cp ${directory}*.ready /media/usb
if [ $? -eq 0  ]
then
	echo "Data copied to disk" | adddate >> /var/log/testlog
else
	echo "Unable to copy data to disk" | adddate >> /var/log/testlog
fi
cp /var/log/vsftpd.log /media/usb
if [ $? -eq 0 ]
then
	echo "FTP log files copied to disk" | adddate >> /var/log/testlog
else
	echo"Unable to copy log files to disk" | adddate >> /var/log/testlog
fi

umount /media/usb

echo "Exiting test program" | adddate >> /var/log/testlog
cp /var/log/testlog /media/usb
exit 0








