#!/bin/bash          
          
echo ST Multiple Cameras Manager
#set variable with path of current directory
DIR="$( cd "$( dirname "$0" )" && pwd )"
#set variable with path of the executable file
PROGR=$DIR"/bin" 
# create a new directory to store images
PARDIR=$DIR"/parfiles"
DATE=`date +%Y-%m-%d.%H-%M-%S`
mkdir -p $DATE
mkdir -p $DATE/NIR
mkdir -p $DATE/VIS
#enter into the new directory
cd $DATE
# launch main camera acquisition streaming program and save screen output to log file 
$PROGR/multiplecam $PARDIR | tee -a log-$DATE.log
 

