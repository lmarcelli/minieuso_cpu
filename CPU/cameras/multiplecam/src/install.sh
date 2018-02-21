#!/bin/bash


DIR=$1
PARDIR=$DIR"parfiles"
PROGR=$DIR"bin" 

if [[ $# -eq 0 ]] ; then
    echo 'No argument passed.'
    echo 'USAGE: ./install.sh SOFTWAREPATH'
    echo 'e.g. ./install.sh /home/software/cameras/multiplecam/'    
    return 1
fi


echo 'Installing multiplecam software'
mkdir -p $PROGR

a="makefile"
if find . -maxdepth 1 -name "$a" -print -quit | grep -q . 
then 
  echo "Found makefile $a. Compiling." 
  make
else 
  echo "Makefile $a is not found. Aborting installation."
  return 1
fi

a="makefile.listserials"
if find . -maxdepth 1 -name "$a" -print -quit | grep -q . 
then 
  echo "Found makefile $a. Compiling." 
  make -f makefile.listserials
else 
  echo "Makefile $a is not found. Aborting installation."
  return 1
fi

a="makefile.singlecam"
if find . -maxdepth 1 -name "$a" -print -quit | grep -q . 
then 
  echo "Found makefile $a. Compiling." 
  make -f makefile.singlecam
else 
  echo "Makefile $a is not found. Aborting installation."
  return 1
fi

echo 'CODE INSTALLED SUCCESSFULLY'

echo 'Now running '$PROGR'/listserials'
$PROGR/listserials $PARDIR/




