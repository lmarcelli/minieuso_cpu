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
#include "DataAcqManager.h"
#include "ThermManager.h"
#include "InputParser.h"

#define VERSION 4.2
#define VERSION_DATE_STRING "19/01/2018"

#define HOME_DIR "/home/software/CPU"
#define DONE_DIR "/home/minieusouser/DONE"
#define DATA_DIR "/home/minieusouser/DATA"

/* functions used in main program */
void SignalHandler(int signum);
void ClearFTP();
int acq_run(UsbManager * UManager, Config * ConfigOut,
		   ZynqManager * ZqManager, DataAcqManager * DaqManager,
		   CamManager * CManager, CmdLineInputs * CmdLine);

#endif
/* _MECONTROL_H */
