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

/* interface to Zynq board */
#define ZYNQ_IP "192.168.7.10"
#define TELNET_PORT 23
#define CONNECT_TIMEOUT_SEC 5

/* pedestal for the ASIC DAC */
#define PEDESTAL 750

class ZynqManager {
public:
  enum InstrumentMode : uint8_t {
    MODE0 = 0,
    MODE1 = 1,
    PERIODIC = 2,
    TRIGGER = 3,
  };
  InstrumentMode instrument_mode;
  enum TestMode : uint8_t {
    T_MODE0 = 0,
    T_MODE1 = 1,
    T_MODE2 = 2,
    T_MODE3 = 3,
    T_MODE4 = 4,
    T_MODE5 = 5,
    T_MODE6 = 6,
  };
  TestMode test_mode;
  enum HvpsStatus : uint8_t {
    OFF = 0,
    ON = 1,
    UNDEF = 2,
  };
  HvpsStatus hvps_status;
  
  ZynqManager();
  static int CheckTelnet();
  static int ConnectTelnet();
  static int GetInstStatus();
  static int GetHvpsStatus();
  int HvpsTurnOn(int cv, int dv);
  int HvpsTurnOff();
  int Scurve(int start, int step, int stop, int acc);
  int SetDac(int dac_level);
  int AcqShot();
  InstrumentMode SetInstrumentMode(InstrumentMode input_mode);
  TestMode SetTestMode(TestMode input_mode);
  static int StopAcquisition();
  int SetNPkts(int N1, int N2);

private:
  static std::string SendRecvTelnet(std::string send_msg, int sockfd);
  static int InstStatusTest(std::string send_msg);

};

#endif /* _ZYNQ_INTERFACE_H */
