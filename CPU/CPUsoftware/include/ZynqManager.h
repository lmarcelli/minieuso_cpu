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
public:
  enum InstrumentMode : uint8_t {
    MODE0 = 0,
    MODE1 = 1,
    MODE2 = 2,
    MODE3 = 3,
  };
  InstrumentMode instrument_mode;
 
  ZynqManager();
  int CheckTelnet();
  int ConnectTelnet();
  int InstStatus();
  int HvpsStatus();
  int HvpsTurnOn(int cv, int dv);
  int Scurve(int start, int step, int stop, int acc);
  int SetDac(int dac_level);
  int AcqShot();
  InstrumentMode SetInstrumentMode(InstrumentMode input_mode);
  /* depreciated commands for compatibility */
#ifdef SINGLE_EVENT
  int DataAcquisitionStart();
  int DataAcquisitionStop();
#endif /* SINGLE_EVENT */

private:
  std::string ip_address; 
  int portno;  
  std::string SendRecvTelnet(std::string send_msg, int sockfd);
  int InstStatusTest(std::string send_msg);

};

#endif /* _ZYNQ_INTERFACE_H */
