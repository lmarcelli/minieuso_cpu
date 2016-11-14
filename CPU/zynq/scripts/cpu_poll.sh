#!/bin/bash

directory='/home/ftp-home/minieusouser/DATA/'
interval=5


while true
do

	mv ${directory}*.ready /home/ftp-home/minieusouser/DONE > /dev/null 2>&1;
	sleep ${interval}

done


