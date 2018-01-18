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

#define VERSION 3.1
#define VERSION_DATE_STRING "26/10/2017"

#define HOME_DIR "/home/software/CPU"
#define DONE_DIR "/home/minieusouser/DONE"
#define DATA_DIR "/home/minieusouser/DATA"

/* number of seconds CPU waits for other systems to boot */
#define BOOT_TIME 4

/* functions used in main program */
void SignalHandler(int signum);
void ClearFTP();
int acq_run(UsbManager * UManager, Config * ConfigOut,
		   ZynqManager * ZqManager, DataAcqManager * DaqManager,
		   CamManager * CManager, CmdLineInputs * CmdLine);

#endif
/* _MECONTROL_H */
