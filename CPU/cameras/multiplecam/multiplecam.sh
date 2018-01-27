#!/bin/bash          
          
echo ST Multiple Cameras Manager
#set variable with path of current directory
#DIR="$( cd "$( dirname "$0" )" && pwd )"

#set DIR manually (FC)
DIR="/home/software/CPU/cameras/multiplecam"

#set variable with path of the executable file
PROGR=$DIR"/bin" 
PARDIR=$DIR"/parfiles"
FILE=check
a="currentNIR.ini"
if find $PARDIR/. -maxdepth 1 -name "$a" -print -quit | grep -q . 
then 
  echo "Found parfile $a" 
  file_status_nir="Y" 
  awk 'x[$1]++ == 1 { print "Warning: " $1 " is duplicated"}' $PARDIR/$a > $FILE 
  if [[ -s $FILE ]]
  then 
  cat $FILE 
  echo "$a not passed security check"
  file_status_nir="N" 
else 
echo "parfile $a passed security check" 
fi  
rm -fr $FILE
else 
  echo "parfile $a is not found. Using default parfile."
  file_status_nir="N"
fi
b="currentVIS.ini"
if find $PARDIR/. -maxdepth 1 -name "$b" -print -quit | grep -q . 
then
  echo "Found parfile $b" 
  file_status_vis="Y"
  awk 'x[$1]++ == 1 { print "Warning: " $1 " is duplicated"}' $PARDIR/$b > $FILE
 if [[ -s $FILE ]] 
 then 
 cat $FILE 
 echo "$b not passed security check"
 file_status_nir="N" 
 else 
 echo "parfile $b passed checks" 
 fi 
 rm -fr $FILE
else 
  echo "parfile $b is not found. Using default parfile."
  file_status_vis="N"
fi
echo "FILE STATUS NIR $file_status_nir VIS $file_status_vis"
# create a new directory to store images
DATE=`date +%Y-%m-%d.%H-%M-%S`
mkdir -p $DIR"/"$DATE
mkdir -p $DIR"/"$DATE/NIR
mkdir -p $DIR"/"$DATE/VIS


#enter into the new directory
cd $DATE
# launch main camera acquisition streaming program and save screen output to log file 
$PROGR/multiplecam $PARDIR $file_status_nir $file_status_vis | tee -a $DIR"/"$DATE"/"log-$DATE.log
 

