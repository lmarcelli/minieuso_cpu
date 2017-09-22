#ifndef _ZYNQ_INTERFACE_H
#define _ZYNQ_INTERFACE_H

#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <fstream>

#include "log.h"

#define ZYNQ_IP "192.168.7.10"
#define TELNET_PORT 23

/* functions for the zynq interface */
/*----------------------------------*/
int CheckTelnet(std::string ip_address, int portno);
std::string SendRecvTelnet(std::string send_msg, int sockfd);
int ConnectTelnet(std::string ip_address, int portno);
int InstStatus();
int InstStatusTest(std::string ip_address, int portno, std::string send_msg);
int HvpsStatus();
int HvpsTurnOn(int cv, int dv);
int Scurve(int start, int step, int stop, int acc);
int DataAcquisitionStart();
int DataAcquisitionStop();
int SetDac(int dac_level);
int AcqShot();

#endif
/* _ZYNQ_INTERFACE_H */
