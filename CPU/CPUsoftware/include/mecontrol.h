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

#define VERSION 2.5
#define VERSION_DATE_STRING "03/10/2017"

#define HOME_DIR "/home/software/CPU"
#define DONE_DIR "/home/minieusouser/DONE"
#define DATA_DIR "/home/minieusouser/DATA"

void SignalHandler(int signum);

#endif
/* _MECONTROL_H */
