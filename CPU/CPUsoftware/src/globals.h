#ifndef _GLOBALS_H
#define _GLOBALS_H

#define VERSION 1.0
#define VERSION_DATE_STRING "09/12/2016"

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
#include <netinet/in.h>
#include <arpa/inet.h>

#include "CPU_functions.h"
#include "general_functions.h"

/* definitions */
#define USB_PORT "/dev/sdb2"
#define ZYNQ_IP_ADDRESS "192.168.7.10"

/* external variables (defined in globals.c) */
extern FILE * log_file;

#endif