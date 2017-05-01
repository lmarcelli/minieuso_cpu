#ifndef _GLOBALS_H
#define _GLOBALS_H

#define VERSION 1.5
#define VERSION_DATE_STRING "01/05/2017"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
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

#include "cpu_functions.h"
#include "general.h"
#include "log.h"
#include "pdmdata.h"
#include "data_format.h"

/* cpu definitions */
#define HOME_DIR "/home/software/CPU"
#define CONFIG_FILE_USB "/media/usb/main_cpu.conf"
#define CONFIG_FILE_LOCAL "/home/software/CPU/CPUsoftware/config/main_cpu.conf"
#define ZYNQ_IP "192.168.7.10"
#define TELNET_PORT 23
/* for testing in stockholm */
//#define ZYNQ_IP "172.29.110.236"
//#define TELNET_PORT 5003
#define DONE_DIR "/home/minieusouser/DONE"
#define DATA_DIR "/home/minieusouser/DATA"

/* external variables */
extern std::string log_name;

#endif
