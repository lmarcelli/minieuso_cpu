#ifndef _CPU_FUNCTIONS_H
#define _CPU_FUNCTIONS_H

#include "config.h"

Config * Parse(std::string config_file_local);
Config * Configure(std::string config_file, std::string config_file_local);
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
