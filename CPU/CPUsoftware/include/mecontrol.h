#ifndef _MECONTROL_H
#define _MECONTROL_H

#include <stdlib.h>
#include <dirent.h>
#include <string>
#include <algorithm>
#include <vector>
#include <thread>

#include "log.h"
#include "configuration.h"
#include "zynq_interface.h"
#include "usb_interface.h"
#include "data_acq.h"

#define VERSION 2.0
#define VERSION_DATE_STRING "21/09/2017"

#define HOME_DIR "/home/software/CPU"
#define CONFIG_FILE_USB "/media/usb/main_cpu.conf"
#define CONFIG_FILE_LOCAL "/home/software/CPU/CPUsoftware/config/main_cpu.conf"
#define DONE_DIR "/home/minieusouser/DONE"
#define DATA_DIR "/home/minieusouser/DATA"
#define LOG_DIR "/home/software/CPU/CPUsoftware/log"
#define CONFIG_DIR "/home/software/CPU/CPUsoftware/config"

#endif
/* _MECONTROL_H */
