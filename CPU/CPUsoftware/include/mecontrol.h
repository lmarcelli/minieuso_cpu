#ifndef _MECONTROL_H
#define _MECONTROL_H

#include <stdlib.h>
#include <dirent.h>
#include <string>
#include <algorithm>
#include <vector>

#include "log.h"
#include "configuration.h"
#include "zynq_interface.h"
#include "usb_interface.h"

/* definition to select event mode */
//#define SINGLE_EVENT
#ifdef SINGLE_EVENT
#include "data_acq_se.h"
#else
#include "data_acq.h"
#endif

#define VERSION 2.0
#define VERSION_DATE_STRING "21/09/2017"

#define HOME_DIR "/home/software/CPU"
#define DONE_DIR "/home/minieusouser/DONE"
#define DATA_DIR "/home/minieusouser/DATA"

void SignalHandler(int signum);

#endif
/* _MECONTROL_H */
