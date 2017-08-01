#ifndef _GLOBALS_H
#define _GLOBALS_H

#define VERSION 1.9
#define VERSION_DATE_STRING "01/08/2017"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/inotify.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <algorithm>
#include <thread>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>
#include <streambuf>
#include <csignal>

#include "dm75xx_library.h"
#include "cpu_functions.h"
#include "general.h"
#include "usb_interface.h"
#include "log.h"
#include "pdmdata.h"
#include "data_format.h"

/* cpu definitions */
#define HOME_DIR "/home/software/CPU"
#define CONFIG_FILE_USB "/media/usb/main_cpu.conf"
#define CONFIG_FILE_LOCAL "/home/software/CPU/CPUsoftware/config/main_cpu.conf"
#define ZYNQ_IP "192.168.7.10"
#define TELNET_PORT 23
#define DONE_DIR "/home/minieusouser/DONE"
#define DATA_DIR "/home/minieusouser/DATA"
#define LOG_DIR "/home/software/CPU/CPUsoftware/log"
#define CONFIG_DIR "/home/software/CPU/CPUsoftware/config"
#define USB_MOUNTPOINT_0 "/media/usb0"
#define USB_MOUNTPOINT_1 "/media/usb1"

/* external variables */
extern std::string log_name;

#endif
