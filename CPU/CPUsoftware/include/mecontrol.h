#ifndef _MECONTROL_H
#define _MECONTROL_H

#include <stdlib.h>
#include <dirent.h>
#include <string>
#include <algorithm>
#include <vector>

#include "log.h"
#include "ConfigManager.h"
#include "ZynqManager.h"
#include "UsbManager.h"
#include "CamManager.h"
#include "LvpsManager.h"

/* definition to select event mode */
//#define SINGLE_EVENT
#ifdef SINGLE_EVENT
#include "DataAcqManagerSe.h"
#else
#include "DataAcqManager.h"
#endif

#define VERSION 3.0
#define VERSION_DATE_STRING "05/10/2017"

#define HOME_DIR "/home/software/CPU"
#define DONE_DIR "/home/minieusouser/DONE"
#define DATA_DIR "/home/minieusouser/DATA"

void SignalHandler(int signum);
int single_acq_run(UsbManager * UManager, Config * ConfigOut,
		   ZynqManager * ZqManager, DataAcqManager * DaqManager,
		   CamManager * CManager, bool hv_on, bool trig_on, bool cam_on, bool sc_on) {

#endif
/* _MECONTROL_H */
