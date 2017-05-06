#ifndef _GENERAL_FUNCTIONS_H
#define _GENERAL_FUNCTIONS_H

#include "pdmdata.h"
#include "data_format.h"

/* for use with inotify in ProcessIncomingData() */
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

/* for use with analog readout functions */
#define CHANNELS 16
#define FIF0_DEPTH 64
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
Z_DATA_TYPE_SCI_POLY_V5 ZynqPktReadOut(std::string zynq_packet);
AnalogAcq AnalogDataCollect();
HK_PACKET AnalogPktReadOut(AnalogAcq acq_output);
int WriteCpuPkt(Z_DATA_TYPE_SCI_POLY_V5 zynq_packet_in, HK_PACKET hk_packet_in, std::string cpu_file_name);
void ProcessIncomingData(std::string cpu_file_name);
int PhotodiodeTest();

#endif

