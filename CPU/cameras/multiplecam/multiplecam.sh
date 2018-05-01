#!/bin/bash
     
##set -x             
echo 'ST Mini-EUSO Additional Cameras Manager'
#set variable with paths
DIR=$1
PROGR=$DIR"bin" 
PARDIR=$DIR"parfiles"
IMAGEDIR=$2
DATE=`date +%Y-%m-%d.%H-%M-%S`
IMAGESAVEDIR=$IMAGEDIR$DATE

if [[ $# -eq 0 ]] ; then
    echo 'No argument passed. Aborting.'
    echo 'USAGE: ./multiplecam.sh SOTFWAREPATH IMAGESAVINGPATH NIR_ON_OFF VIS_ON_OFF'
    echo 'e.g. to start acquisition with both cameras, run:' 
    echo './multiplecam.sh /home/software/cameras/multiplecam/ /media/usb0/ 1 1'    
    return 1
fi
if [ $# -le 3 ]
then
  echo 'Not enough arguments passed. Aborting.'
  echo 'USAGE: ./multiplecam.sh SOTFWAREPATH IMAGESAVINGPATH NIR_ON_OFF VIS_ON_OFF'
  echo 'e.g. to start acquisition with both cameras, run:' 
  echo './multiplecam.sh /home/software/cameras/multiplecam/ /media/usb0/ 1 1'  
  return 1
fi

case $3 in
0*)
echo 'WARNING: You asked not to connect the NIR camera'
CAMNAME=VIS; # to connect the other camera for the serial number
;;
1*)
echo 'You requested to connect the NIR camera'
# create a new directory to store images
mkdir -p $IMAGESAVEDIR
mkdir -p $IMAGESAVEDIR/NIR
echo "Saving NIR images in: $IMAGESAVEDIR/NIR"
#check parfiles
FILE=check
a="currentNIR.ini"
if find $PARDIR/. -maxdepth 1 -name "$a" -print -quit | grep -q . 
then 
  echo "Found parfile $a" 
  file_status_nir="Y"
  file_status="Y" 
  awk 'x[$1]++ == 1 { print "Warning: " $1 " is duplicated"}' $PARDIR/$a > $FILE 
  if [[ -s $FILE ]]
  then 
  cat $FILE 
  echo "$a not passed security check"
  file_status_nir="N" 
  file_status="N" 
else 
echo "parfile $a passed security check" 
fi  
rm -fr $FILE
else 
  echo "parfile $a is not found. Using default parfile."
  file_status_nir="N"
  file_status="N"
fi
;;
*)
echo 'Something is not quite right here... Check your input variable #3. Forced Exit. '
      return 1
;;
esac

case $4 in
0*)
echo 'WARNING: You asked not to connect the VIS camera'
CAMNAME=NIR; # to connect the other camera for the serial number
;;
1*)
echo 'You requested to connect the VIS camera'
# create a new directory to store images
mkdir -p $IMAGESAVEDIR
mkdir -p $IMAGESAVEDIR/VIS
echo "Saving VIS images in: $IMAGESAVEDIR/VIS"
b="currentVIS.ini"
if find $PARDIR/. -maxdepth 1 -name "$b" -print -quit | grep -q . 
then
  echo "Found parfile $b" 
  file_status_vis="Y"
  file_status="Y"
  awk 'x[$1]++ == 1 { print "Warning: " $1 " is duplicated"}' $PARDIR/$b > $FILE
 if [[ -s $FILE ]] 
 then 
 cat $FILE 
 echo "$b not passed security check"
 file_status_vis="N" 
 file_status="N" 
 else 
 echo "parfile $b passed checks" 
 fi 
 rm -fr $FILE
else 
  echo "parfile $b is not found. Using default parfile."
  file_status_vis="N"
  file_status="N" 
fi
;;
*)
echo 'Something is not quite right here... Check your input variable #4. Forced Exit. '   
      return 1
;;
esac

CAMNUMBER=$(($3 + $4))
case $CAMNUMBER in
0*)
echo 'You are asking to connect no camera. Forced Exit.'
    return 1
;;
1*)
echo 'WARNING: You are asking to connect only one camera'
echo "FILE STATUS $file_status"
echo "Launching single camera acquisition streaming program $PROGR/singlecam" 
echo "Screen output is saved to log file $IMAGESAVEDIR/log-$DATE.log" 
ser="cameras.ini"
if find $PARDIR/. -maxdepth 1 -name "$ser" -print -quit | grep -q . 
then 
  echo "Found $PARDIR/$ser file"
  while read -r f1 f2 f3
do
        # display fields using f1, f2, f3
###        printf 'Serial: %s, Type: %s, Model: %s\n' "$f1" "$f2" "$f3" ### uncomment for debugging  
        if [ "$f2" == "$CAMNAME" ]
        then              
             SERIAL=$f1
###             printf "$SERIAL\n" ### uncomment for debugging  
        fi
done <"$PARDIR/$ser"
else
 echo "The file $PARDIR/$ser is not found. Please create it. Aborting now."
 return 1
fi  
echo "Launching camera acquisition streaming program $PROGR/singlecam" 
echo "Screen output is saved to log file $IMAGESAVEDIR/log-$DATE.log"
# launch camera acquisition streaming program singlecam and save screen output to log file 
$PROGR/singlecam $PARDIR $file_status $IMAGESAVEDIR $SERIAL |& tee -a $IMAGESAVEDIR/log-$DATE.log
;;
2*)
echo 'You are asking to connect both cameras'
echo "FILE STATUS NIR $file_status_nir VIS $file_status_vis"
echo "Launching main camera acquisition streaming program $PROGR/multiplecam" 
echo "Screen output is saved to log file $IMAGESAVEDIR/log-$DATE.log"
# launch main camera acquisition streaming program multiplecam and save screen output to log file 
$PROGR/multiplecam $PARDIR $file_status_nir $file_status_vis $IMAGESAVEDIR |& tee -a $IMAGESAVEDIR/log-$DATE.log
;;
*)
echo 'Something is not quite right here... Check your input variables. Forced Exit. '
      return 1
;;
esac

