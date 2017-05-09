#ifndef _GENERAL_FUNCTIONS_H
#define _GENERAL_FUNCTIONS_H

#include "pdmdata.h"
#include "data_format.h"
#include "config.h"

/* for use with inotify in ProcessIncomingData() */
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

/* for use with analog readout functions */
#define CHANNELS 16
#define FIFO_DEPTH 64
#define BURST_RATE 1000000
#define PACER_RATE 100000
#define PH_CHANNELS 4

/* acquisition structure for analog readout */
typedef struct
{
  float val [FIFO_DEPTH][CHANNELS];
} AnalogAcq;

std::string CreateLogname(void);
bool CopyFile(const char * SRC, const char * DEST);
void SignalHandler(int signum);
std::string CreateCpuRunName(void);
int CreateCpuRun(std::string cpu_file_name);
SCURVE_PACKET ScPktReadOut(std::string sc_file_name, const Config& ConfigOut);
Z_DATA_TYPE_SCI_POLY_V5 ZynqPktReadOut(std::string zynq_file_name);
AnalogAcq AnalogDataCollect();
HK_PACKET AnalogPktReadOut(AnalogAcq acq_output);
int WriteScPkt(SCURVE_PACKET sc_packet_in, std::string cpu_file_name);
int WriteCpuPkt(Z_DATA_TYPE_SCI_POLY_V5 zynq_packet_in, HK_PACKET hk_packet_in, std::string cpu_file_name);
int ProcessIncomingData(std::string cpu_file_name, Config * pConfigOut);
void ProcessIncomingData_SF(std::string cpu_file_name, const Config& ConfigOut);

#endif

