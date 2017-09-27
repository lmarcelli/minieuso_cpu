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
#include <algorithm>

#include "log.h"

#define ZYNQ_IP "192.168.7.10"
#define TELNET_PORT 23

class ZynqManager {
private:
  std::string ip_address; 
  int portno;  

  std::string SendRecvTelnet(std::string send_msg, int sockfd);
  int InstStatusTest(std::string send_msg);

public:
  ZynqManager();
  int CheckTelnet();
  int ConnectTelnet();
  int InstStatus();
  int HvpsStatus();
  int HvpsTurnOn(int cv, int dv);
  int Scurve(int start, int step, int stop, int acc);
  int DataAcquisitionStart();
  int DataAcquisitionStop();
  int SetDac(int dac_level);
  int AcqShot();
  
};

#endif
/* _ZYNQ_INTERFACE_H */
