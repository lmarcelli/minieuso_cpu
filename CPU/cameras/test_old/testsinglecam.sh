#!/bin/bash          
          
echo ST SPEED TEST Single Camera
PROGR="/home/minieusouser/CPU/cameras/test/bin" 
# create a new directory to store images
DATE=`date +%Y-%m-%d.%H-%M-%S`
mkdir -p $DATE
cd $DATE
# launch main data acquisition program
$PROGR/noverbose  
 

