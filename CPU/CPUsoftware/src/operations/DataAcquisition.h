#ifndef _DATA_ACQUISITION_H
#define _DATA_ACQUISITION_H

#ifndef __APPLE__
#include <sys/inotify.h>
#endif /* __APPLE__ */
#include <thread>

#include "OperationMode.h"
#include "ThermManager.h"
#include "AnalogManager.h"
#include "pdmdata.h"
#include "InputParser.h"

#define DATA_DIR "/home/minieusouser/DATA"
#define DONE_DIR "/home/minieusouser/DONE"
#define USB_MOUNTPOINT_0 "/media/usb0"
#define USB_MOUNTPOINT_1 "/media/usb1"

/* for use with inotify in ProcessIncomingData() */
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

/* NIGHT operational mode: data acquisition */
/* class for controlling the main acquisition */
/* (the Zynq board, the thermistors and the Analog board) */
class DataAcquisition : public OperationMode {   
public:  
  std::string cpu_main_file_name;
  std::string cpu_sc_file_name;
  std::string cpu_hv_file_name;
  uint8_t usb_num_storage_dev;
  
  /* synchronised file access */
  std::shared_ptr<SynchronisedFile> CpuFile;
  Access * RunAccess;

  /* subsystems controlled */
  ThermManager * ThManager = new ThermManager();
  AnalogManager * Analog = new AnalogManager();
  
  enum RunType : uint8_t {
    CPU = 0,
    SC = 1,
    HV = 2,
  };

  DataAcquisition();
  int CreateCpuRun(RunType run_type, Config * ConfigOut);
  int CloseCpuRun(RunType run_type);
  int CollectSc(ZynqManager * ZqManager, Config * ConfigOut, CmdLineInputs * CmdLine);
  int CollectData(ZynqManager * ZqManager, Config * ConfigOut, CmdLineInputs * CmdLine);
  static int WriteFakeZynqPkt();
  static int ReadFakeZynqPkt();
  
private:  
  std::string CreateCpuRunName(RunType run_type, Config * ConfigOut);
  static uint32_t BuildCpuFileHeader(uint32_t type, uint32_t ver);
  static uint32_t BuildCpuPktHeader(uint32_t type, uint32_t ver);
  static uint32_t BuildCpuTimeStamp();
  SC_PACKET * ScPktReadOut(std::string sc_file_name, Config * ConfigOut);
  HV_PACKET * HvPktReadOut(std::string hv_file_name);
  ZYNQ_PACKET * ZynqPktReadOut(std::string zynq_file_name, Config * ConfigOut);
  HK_PACKET * AnalogPktReadOut();
  int WriteScPkt(SC_PACKET * sc_packet);
  int WriteHvPkt(HV_PACKET * hv_packet);
  int WriteCpuPkt(ZYNQ_PACKET * zynq_packet, HK_PACKET * hk_packet, Config * ConfigOut);
  int ProcessIncomingData(Config * ConfigOut, CmdLineInputs * CmdLine);
  
};

#endif
/* _DATA_ACQUISITION_H */
